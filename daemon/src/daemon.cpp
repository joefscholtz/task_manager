#include "daemon.hpp"

namespace task_manager {
Daemon *Daemon::s_instance = nullptr;

Daemon::Daemon() {
  s_instance = this;
  this->populate_dbus_callbacks();
  openlog("task_manager_daemon", LOG_PID, LOG_DAEMON);
  this->create_pid_file();

  signal(SIGINT, &Daemon::handle_signal);
  signal(SIGTERM, &Daemon::handle_signal);

  // Registering with atexit is a robust way to ensure cleanup happens
  // even if the program exits unexpectedly (but not from a hard crash).
  atexit([] {
    if (s_instance) {
      s_instance->remove_pid_file();
    }
  });
}

Daemon::~Daemon() {
  this->remove_pid_file();
  this->close_log();
}

void Daemon::handle_signal(int signal) {
  if (s_instance) {
    syslog(LOG_INFO, "Shutdown signal received. Cleaning up...");
    s_instance->set_state(Daemon::STATE::SHUTTING_DOWN);
    s_instance->_shutdown_requested = true;
    s_instance->remove_pid_file();
  }
}

void Daemon::init_dbus() {
  pugi::xml_document doc;
  const char *xml_file_path = "../config/org.task_manager.Daemon.xml";
  try {
    if (!doc.load_file(xml_file_path)) {
      syslog(LOG_ERR,
             "D-Bus: Failed to load or parse interface XML file at %s.",
             xml_file_path);
      return;
    }

    pugi::xml_node interface_node = doc.child("node").child("interface");
    if (!interface_node) {
      syslog(LOG_ERR, "D-Bus: Could not find <interface> node in XML.");
      return;
    }

    std::string serviceName = interface_node.attribute("name").value();
    if (serviceName.empty()) {
      syslog(LOG_ERR,
             "D-Bus: <interface> tag is missing a 'name' attribute in XML.");
      return;
    }

    // The objectPath is derived by replacing '.' with '/' and prepending '/'.
    // e.g., "org.task_manager.Daemon" -> "/org/task_manager/Daemon"
    std::string objectPath =
        "/" + std::regex_replace(serviceName, std::regex("\\."), "/");

    syslog(LOG_INFO, "D-Bus: Initializing service '%s' at path '%s'",
           serviceName.c_str(), objectPath.c_str());

    this->_dbus_connection = sdbus::createSessionBusConnection(serviceName);
    this->_dbus_object =
        sdbus::createObject(*this->_dbus_connection, objectPath);

    for (pugi::xml_node method_node : interface_node.children("method")) {
      std::string method_name = method_node.attribute("name").value();

      auto it = _dispatch_table.find(method_name);
      if (it != _dispatch_table.end()) {
        this->_dbus_object->registerMethod(method_name)
            .onInterface(serviceName)
            .implementedAs(it->second);
        syslog(LOG_INFO, "D-Bus: Registered method '%s'", method_name.c_str());
      } else {
        syslog(LOG_WARNING,
               "D-Bus: Method '%s' found in XML but has no C++ implementation.",
               method_name.c_str());
      }
    }

    this->_dbus_object->finishRegistration();
  } catch (const std::exception &e) {
    syslog(LOG_ERROR, "An unhandled exception occurred: '%s'", e.what());
    return 1;
  } catch (...) {
    syslog(LOG_ERROR, "An unhandled exception occurred");
    return 1;
  }
}

void Daemon::run() {
  syslog(LOG_INFO, "Daemon starting up...");

  // The main lifecycle loop
  while (this->_state != Daemon::State::STOPPED && !this->_shutdown_requested) {
    switch (this->_state) {
    case Daemon::State::STARTING:
      syslog(LOG_INFO, "Daemon::State: STARTING -> CONFIGURING");
      // Minimal setup before configuration
      this->_state = Daemon::State::CONFIGURING;
      break;

    case Daemon::State::CONFIGURING:
      syslog(LOG_INFO, "Daemon::State: CONFIGURING -> ACTIVE");
      // Load settings, initialize Calendar, connect to DB
      this->init_dbus();
      this->_state = Daemon::State::ACTIVE;
      break;

    case Daemon::State::ACTIVE:
      // This is the main operational state.
      // It will block here until a request comes in or the loop is interrupted.
      this->_dbus_connection->processRequests();
      break;

    case Daemon::State::SHUTTING_DOWN:
      syslog(LOG_INFO, "State: SHUTTING_DOWN -> STOPPED");
      // Clean up resources
      this->_state = Daemon::State::STOPPED;
      break;

    case Daemon::State::STOPPED:
      // Loop will terminate
      break;
    }
  }

  // If the loop was exited because a signal was received
  if (_shutdown_requested && _state != Daemon::State::STOPPED) {
    syslog(LOG_INFO, "Shutdown requested. Finalizing...");
    // Perform final cleanup
  }
}

std::string get_pid_file_path() {
  // Storing the PID file in the user's runtime directory is the modern
  // standard. It's secure and is often automatically cleaned up on logout.
  std::string path = "/var/run/user/" + std::to_string(getuid());

  // Ensure the directory exists
  std::filesystem::create_directories(path);

  return path + "/task_manager.pid";
}

void Daemon::create_pid_file() {
  const std::string pid_path = get_pid_file_path();

  std::ifstream pid_file_in(pid_path);
  if (pid_file_in.is_open()) {
    pid_t old_pid;
    pid_file_in >> old_pid;
    pid_file_in.close();

    if (kill(old_pid, 0) == 0) {
      syslog(LOG_ERR, "Daemon is already running with PID %d. Exiting.",
             old_pid);
      exit(EXIT_FAILURE);
    } else {
      syslog(LOG_WARNING, "Found a stale PID file for PID %d. Overwriting.",
             old_pid);
    }
  }

  // Write the current process ID to the file.
  std::ofstream pid_file_out(pid_path);
  if (pid_file_out.is_open()) {
    pid_file_out << getpid() << std::endl;
  } else {
    syslog(LOG_ERR, "Could not create PID file at %s. Exiting.",
           pid_path.c_str());
    exit(EXIT_FAILURE);
  }
}
void Daemon::remove_pid_file() {
  const std::string pid_path = get_pid_file_path();
  if (std::remove(pid_path.c_str()) == 0) {
    syslog(LOG_INFO, "PID file removed successfully.");
  } else {
    syslog(LOG_WARNING, "Could not remove PID file. It may need to be removed "
                        "manually or it was already removed.");
  }
}

bool Daemon::populate_dbus_callbacks() {
  _dispatch_table["GetEventsForMonth"] =
      [this](const std::vector<sdbus::Variant> &args) -> sdbus::Variant {
    // Unpack variants and call the real method
    int32_t year = args[0].get<int32_t>();
    int32_t month = args[1].get<int32_t>();
    return this->GetEventsForMonth(year, month);
  };

  _dispatch_table["SyncAllAccounts"] =
      [this](const std::vector<sdbus::Variant> &args) -> sdbus::Variant {
    this->SyncAllAccounts();
    return {}; // Return an empty variant for void methods
  };
}

std::string Daemon::GetEventsForMonth(const int32_t &year,
                                      const int32_t &month) {
  // TODO: Call _calendar logic here to get events and serialize to JSON
  syslog(LOG_INFO, "GetEventsForMonth called for %d-%d", year, month);
  nlohmann::json response = {{"status", "success"},
                             {"data", {"event1", "event2"}}};
  return response.dump();
}

void Daemon::SyncAllAccounts() {
  syslog(LOG_INFO, "Manual sync triggered via D-Bus.");
  bool success = this->_calendar.sync_external_events();

  // Emit a signal to notify any listening clients
  this->_dbus_object->emitSignal("SyncCompleted")
      .onInterface("org.task_manager.Daemon")
      .withArguments(success);
}

} // namespace task_manager

#include "lifecycle_daemon.hpp"

namespace task_manager {
LifecycleDaemon *LifecycleDaemon::s_instance = nullptr;

LifecycleDaemon::LifecycleDaemon() {
  s_instance = this;

  DbusConfig config = parse_dbus_config();
  this->_dbus_connection =
      sdbus::createSessionBusConnection(config.service_name);
  this->_daemon = std::make_unique<Daemon>(*(this->_dbus_connection), config);

  openlog("task_manager_daemon", LOG_PID, LOG_DAEMON);
  this->create_pid_file();

  // TODO: which is the best approach?
  signal(SIGINT, &LifecycleDaemon::handle_signal);
  signal(SIGTERM, &LifecycleDaemon::handle_signal);
  // Registering with atexit is a robust way to ensure cleanup happens
  // even if the program exits unexpectedly (but not from a hard crash).
  atexit([] {
    if (s_instance) {
      s_instance->remove_pid_file();
    }
  });
}

LifecycleDaemon::~LifecycleDaemon() {
  this->remove_pid_file();
  closelog();
}

void LifecycleDaemon::handle_signal(int signal) {
  if (s_instance) {
    syslog(LOG_INFO, "Shutdown signal received. Cleaning up...");
    this->_daemon->set_state(Daemon::STATE::SHUTTING_DOWN);
    this->_daemon->_shutdown_requested = true;
    this->_daemon->remove_pid_file();
  }
}

void LifecycleDaemon::run() {
  syslog(LOG_INFO, "Daemon starting up...");

  // The main lifecycle loop
  while (this->_daemon._state != Daemon::State::STOPPED &&
         !this->_shutdown_requested) {
    switch (this->_daemon._state) {
    case Daemon::State::STARTING:
      syslog(LOG_INFO, "Daemon::State: STARTING -> CONFIGURING");
      // Minimal setup before configuration
      this->_daemon->populate_dbus_callbacks();
      this->_daemon._state = Daemon::State::CONFIGURING;
      break;

    case Daemon::State::CONFIGURING:
      syslog(LOG_INFO, "Daemon::State: CONFIGURING -> ACTIVE");
      // Load settings, initialize Calendar, connect to DB
      this->_daemon->init_dbus();
      this->_daemon._state = Daemon::State::ACTIVE;
      break;

    case Daemon::State::ACTIVE:
      // This is the main operational state.
      // It will block here until a request comes in or the loop is interrupted.
      this->_daemon._dbus_connection->processRequests();
      break;

    case Daemon::State::SHUTTING_DOWN:
      syslog(LOG_INFO, "State: SHUTTING_DOWN -> STOPPED");
      // Clean up resources
      this->_daemon._state = Daemon::State::STOPPED;
      break;

    case Daemon::State::STOPPED:
      // Loop will terminate
      break;
    }
  }
  // If the loop was exited because a signal was received
  if (this->_shutdown_requested &&
      this->_daemon._state != Daemon::State::STOPPED) {
    syslog(LOG_INFO, "Shutdown requested. Finalizing...");
    // Perform final cleanup
  }
}

std::string LifecycleDaemon::get_pid_file_path() {
  // Storing the PID file in the user's runtime directory is the modern
  // standard. It's secure and is often automatically cleaned up on logout.
  std::string path = "/var/run/user/" + std::to_string(getuid());

  // Ensure the directory exists
  std::filesystem::create_directories(path);

  return path + "/task_manager.pid";
}

void LifecycleDaemon::create_pid_file() {
  const std::string pid_path = this->get_pid_file_path();

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
void LifecycleDaemon::remove_pid_file() {
  const std::string pid_path = this->get_pid_file_path();
  if (std::remove(pid_path.c_str()) == 0) {
    syslog(LOG_INFO, "PID file removed successfully.");
  } else {
    syslog(LOG_WARNING, "Could not remove PID file. It may need to be removed "
                        "manually or it was already removed.");
  }
}

static DbusConfig
LifecycleDaemon::parse_dbus_config(const std::string &xml_file_path) {
  pugi::xml_document dbus_interface_xml;
  pugi::xml_node interface_node;
  std::string service_name;
  std::string object_path;
  try {
    if (!dbus_interface_xml.load_file(xml_file_path.c_str())) {
      syslog(LOG_ERR,
             "D-Bus: Failed to load or parse interface XML file at %s.",
             xml_file_path.c_str());
      throw std::runtime_error(
          "D-Bus: Failed to load or parse interface XML file at %s.");
    }
    interface_node = dbus_interface_xml.child("node").child("interface");
    if (!interface_node) {
      syslog(LOG_ERR, "D-Bus: Could not find <interface> node in XML.");
      throw std::runtime_error(
          "D-Bus: Could not find <interface> node in XML.");
    }

    service_name = interface_node.attribute("name").value();
    if (service_name.empty()) {
      syslog(LOG_ERR,
             "D-Bus: <interface> tag is missing a 'name' attribute in XML.");
      throw std::runtime_error(
          "D-Bus: <interface> tag is missing a 'name' attribute in XML.");
    }

    // The objectPath is derived by replacing '.' with '/' and prepending '/'.
    // e.g., "org.task_manager.Daemon" -> "/org/task_manager/Daemon"
    object_path =
        "/" + std::regex_replace(service_name, std::regex("\\."), "/");
  } catch (const std::exception &e) {
    syslog(LOG_ERR, "An unhandled exception occurred: '%s'", e.what());
    return {};
  } catch (...) {
    syslog(LOG_ERR, "An unhandled exception occurred");
    return {};
  }
  return {xml_file_path, interface_node, service_name, object_path};
}

} // namespace task_manager

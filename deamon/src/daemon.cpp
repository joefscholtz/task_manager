#include "daemon.hpp"

namespace task_manager {
Daemon *Daemon::s_instance = nullptr;

Daemon::Daemon() {
  s_instance = this;
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
    s_instance->set_state(STATE::SHUTTING_DOWN);
    s_instance->_shutdown_requested = true;
    this->remove_pid_file();
  }
}

void Daemon::init_dbus() {
  const char *serviceName = "org.task_manager.Daemon";
  this->_connection = sdbus::createSessionBusConnection(serviceName);

  const char *objectPath = "/org/task_manager/Daemon";
  this->_dbus_object = sdbus::createObject(*_connection, objectPath);

  this->_dbus_object->registerMethod("GetEventsForMonth")
      .onInterface(serviceName)
      .implementedAs([this](const int32_t &year, const int32_t &month) {
        return this->GetEventsForMonth(year, month);
      });
  this->_dbus_object->registerMethod("SyncAllAccounts")
      .onInterface(serviceName)
      .implementedAs([this]() { this->SyncAllAccounts(); });

  this->_dbus_object->registerSignal("SyncCompleted")
      .onInterface(serviceName)
      .withArguments<bool>();

  this->_dbus_object->finishRegistration();
}

void Daemon::run() {
  syslog(LOG_INFO, "Daemon starting up...");

  // The main lifecycle loop
  while (this->_state != State::STOPPED && !this->_shutdown_requested) {
    switch (this->_state) {
    case State::STARTING:
      syslog(LOG_INFO, "State: STARTING -> CONFIGURING");
      // Minimal setup before configuration
      this->_state = State::CONFIGURING;
      break;

    case State::CONFIGURING:
      syslog(LOG_INFO, "State: CONFIGURING -> ACTIVE");
      // Load settings, initialize Calendar, connect to DB
      this->init_dbus();
      this->_state = State::ACTIVE;
      break;

    case State::ACTIVE:
      // This is the main operational state.
      // It will block here until a request comes in or the loop is interrupted.
      this->_connection->processRequests();
      break;

    case State::SHUTTING_DOWN:
      syslog(LOG_INFO, "State: SHUTTING_DOWN -> STOPPED");
      // Clean up resources
      this->_state = State::STOPPED;
      break;

    case State::STOPPED:
      // Loop will terminate
      break;
    }
  }

  // If the loop was exited because a signal was received
  if (_shutdown_requested && _state != State::STOPPED) {
    syslog(LOG_INFO, "Shutdown requested. Finalizing...");
    // Perform final cleanup
  }
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
} // namespace task_manager

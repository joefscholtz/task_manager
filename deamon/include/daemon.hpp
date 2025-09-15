#pragma once

#include "defines.hpp"

#include "core.hpp"
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <syslog.h>
#include <unistd.h>

namespace task_manager {
class Daemon {
public:
  enum class State { STARTING, CONFIGURING, ACTIVE, SHUTTING_DOWN, STOPPED };

  Daemon();
  ~Daemon();

  inline const State &get_state() const { return this->_state; }
  inline void set_state(State id) { this->_state = state; }

  void run();

private:
  // D-Bus methods that will be exposed
  std::string GetEventsForMonth(const int32_t &year, const int32_t &month);
  void SyncAllAccounts();

  void init_dbus();
  std::string get_pid_file_path();
  void create_pid_file();
  void remove_pid_file();

  // Signal handling
  static void handle_signal(int signal);
  static Daemon *s_instance;

  task_manager::Calendar _calendar;
  std::unique_ptr<sdbus::IConnection> _dbus_connection;
  std::unique_ptr<sdbus::IObject> _dbus_object;
  bool _shutdown_requested = false;
  State _state = State::STARTING;
};
} // namespace task_manager

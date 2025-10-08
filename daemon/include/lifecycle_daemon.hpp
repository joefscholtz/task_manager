#pragma once

#include "defines.hpp"

#include "daemon.hpp"

namespace task_manager {
class LifecycleDaemon {
public:
  LifecycleDaemon();
  ~LifecycleDaemon();

  void run();

private:
  static LifecycleDaemon *s_instance;

  void init_signals();
  std::string get_pid_file_path();
  void create_pid_file();
  void remove_pid_file();

  static void handle_signal(int signal);

  std::unique_ptr<sdbus::IConnection> _dbus_connection;
  std::unique_ptr<Daemon> _daemon;
  bool _shutdown_requested = false;
};
} // namespace task_manager

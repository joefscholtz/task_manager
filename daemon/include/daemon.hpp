#pragma once

#include "defines.hpp"

#include "core.hpp"
#include "pugixml.hpp"
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <syslog.h>
#include <unistd.h>

#include "daemon_adaptor.h"

namespace task_manager {

// The Daemon class implements the D-Bus interface by inheriting the generated
// adaptor.
class Daemon final
    : public sdbus::AdaptorInterfaces<org::task_manager::Daemon_adaptor> {
public:
  enum class State { STARTING, CONFIGURING, ACTIVE, SHUTTING_DOWN, STOPPED };

  Daemon(sdbus::IConnection &connection, std::string object_path);
  ~Daemon();

  inline const State &get_state() const { return this->_state; }
  inline void set_state(State &state) { this->_state = state; }

private:
  // D-Bus methods that will be exposed
  std::string GetEventsForMonth(const int32_t &year, const int32_t &month);
  void SyncAllAccounts();
  inline std::string Version() { return "1.0.0"; }

  task_manager::Calendar _calendar;
  std::string _object_path;

  State _state = State::STARTING;
};
} // namespace task_manager

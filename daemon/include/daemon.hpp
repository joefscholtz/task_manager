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

namespace task_manager {

using DbusMethodCallback =
    std::function<sdbus::Variant(const std::vector<sdbus::Variant> &)>;

struct DbusConfig {
  std::string xml_file_path;
  pugi::xml_node interface_node;
  std::string service_name;
  std::string object_path;
};

class Daemon final
    : public sdbus::AdaptorInterfaces<sdbus::Properties_adaptor> {
public:
  enum class State { STARTING, CONFIGURING, ACTIVE, SHUTTING_DOWN, STOPPED };

  Daemon(sdbus::IConnection &connection, const DbusConfig &config);
  ~Daemon();

  inline const State &get_state() const { return this->_state; }
  inline void set_state(State &state) { this->_state = state; }

private:
  // D-Bus methods that will be exposed
  std::map<std::string, DbusMethodCallback> _dispatch_table;
  bool populate_dbus_callbacks();
  std::string GetEventsForMonth(const int32_t &year, const int32_t &month);
  void SyncAllAccounts();

  sdbus::Variant getProperty(const std::string &propertyName);
  void setProperty(const std::string &propertyName,
                   const sdbus::Variant &value);
  bool init_dbus();

  task_manager::Calendar _calendar;
  std::string _xml_file_path;
  // pugi::xml_document _dbus_interface_xml;
  pugi::xml_node _interface_node;
  std::string _service_name;
  std::string _object_path;

  std::unique_ptr<sdbus::IObject> _dbus_object;
  State _state = State::STARTING;
};
} // namespace task_manager

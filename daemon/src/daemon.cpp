#include "daemon.hpp"

namespace task_manager {

Daemon::Daemon(sdbus::IConnection &connection, const DbusConfig &config)
    : AdaptorInterfaces(connection, config.object_path),
      _xml_file_path(config.xml_file_path),
      _interface_node(config.interface_node),
      _service_name(config.service_name), _object_path(config.object_path) {
  this->init_dbus();
}

bool Daemon::init_dbus() {
  try {
    this->populate_dbus_callbacks();
    syslog(LOG_INFO, "D-Bus: Initializing service '%s' at path '%s'",
           this->_service_name.c_str(), this->_object_path.c_str());

    for (pugi::xml_node method_node :
         this->_interface_node.children("method")) {
      std::string method_name = method_node.attribute("name").value();

      auto it = _dispatch_table.find(method_name);
      if (it != _dispatch_table.end()) {
        this->registerMethod(method_name)
            .onInterface(this->_service_name)
            .implementedAs(it->second);
        syslog(LOG_INFO, "D-Bus: Registered method '%s'", method_name.c_str());
      } else {
        syslog(LOG_WARNING,
               "D-Bus: Method '%s' found in XML but has no C++ implementation.",
               method_name.c_str());
      }
    }

    this->finishRegistration();
  } catch (const std::exception &e) {
    syslog(LOG_ERR, "An unhandled exception occurred: '%s'", e.what());
    return false;
  } catch (...) {
    syslog(LOG_ERR, "An unhandled exception occurred");
    return false;
  }
  return true;
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
  return true;
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
  this->emitSignal("SyncCompleted")
      .onInterface("org.task_manager.Daemon")
      .withArguments(success);
}

} // namespace task_manager

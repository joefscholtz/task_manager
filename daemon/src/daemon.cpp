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
    syslog(LOG_INFO, "D-Bus: Initializing service '%s' at path '%s'",
           this->_service_name.c_str(), this->_object_path.c_str());

    for (pugi::xml_node node : this->_interface_node.children()) {
      std::string node_name = node.name();

      if (node_name == "method") {
        std::string method_name = node.attribute("name").value();
        if (method_name == "GetEventsForMonth") {
          registerMethod(method_name)
              .onInterface(this->_service_name)
              .implementedAs([this](const int32_t &year, const int32_t &month) {
                return this->GetEventsForMonth(year, month);
              });
          syslog(LOG_INFO, "  - Registered Method: %s", method_name.c_str());
        } else if (method_name == "SyncAllAccounts") {
          registerMethod(method_name)
              .onInterface(this->_service_name)
              .implementedAs([this]() { this->SyncAllAccounts(); });
          syslog(LOG_INFO, "  - Registered Method: %s", method_name.c_str());
        } else {
          syslog(LOG_WARNING,
                 "  - WARNING: Method '%s' from XML has no C++ implementation.",
                 method_name.c_str());
        }
      } else if (node_name == "signal") {
        std::string signal_name = node.attribute("name").value();
        registerSignal(signal_name).onInterface(config.service_name);
        syslog(LOG_INFO, "  - Registered Signal: %s", signal_name.c_str());
      } else if (node_name == "property") {
        std::string prop_name = node.attribute("name").value();
        std::string access = node.attribute("access").value();

        auto prop =
            registerProperty(prop_name).onInterface(config.service_name);

        if (access == "read" || access == "readwrite") {
          prop.withGetter(
              [this, prop_name]() { return this->getProperty(prop_name); });
        }
        if (access == "write" || access == "readwrite") {
          prop.withSetter([this, prop_name](const sdbus::Variant &value) {
            this->setProperty(prop_name, value);
          });
        }
        syslog(LOG_INFO, "  - Registered Property: %s (access: %s)",
               prop_name.c_str(), access.c_str());
      }
    }
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

sdbus::Variant Daemon::getProperty(const std::string &propertyName) {
  if (propertyName == "Version") {
    return sdbus::Variant("1.0.0");
  }
  return sdbus::Variant("");
}

void Daemon::setProperty(const std::string &propertyName,
                         const sdbus::Variant &value) {
  syslog(LOG_INFO, "Attempted to set property '%s'", propertyName.c_str());
}

} // namespace task_manager

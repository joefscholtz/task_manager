#include "daemon.hpp"

namespace task_manager {

Daemon::Daemon(sdbus::IConnection &connection, std::string object_path)
    : AdaptorInterfaces(connection, object_path), _object_path(object_path) {
  registerAdaptor();
}

Daemon::~Daemon() { unregisterAdaptor(); }

std::string Daemon::GetEventsForMonth(const int32_t &year,
                                      const int32_t &month) {
  // TODO: Call _calendar logic here to get events and serialize to JSON
  syslog(LOG_INFO, "GetEventsForMonth called for %d-%d", year, month);
  nlohmann::json response = {{"status", "success"}, {"data", {}}};
  return response.dump();
}

void Daemon::SyncAllAccounts() {
  syslog(LOG_INFO, "Manual sync triggered via D-Bus.");
  bool success = this->_calendar.sync_external_events();

  // Emit a signal to notify any listening clients
  emitSyncCompleted(success);
}
} // namespace task_manager

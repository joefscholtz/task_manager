#include "defines.hpp"

#include "daemon.hpp"
#include "lifecycle_daemon.hpp"

using namespace task_manager;

int main(int argc, char *argv[]) {
  try {
    LifecycleDaemon lifecycle_daemon;
    lifecycle_daemon.run();
  } catch (const sdbus::Error &e) {
    syslog(LOG_ERROR, "An unhandled DBUS exception occurred: '%s'", e.what());
    return 1;
  }
  return 0;
}

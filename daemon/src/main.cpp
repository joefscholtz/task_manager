#include "defines.hpp"

#include "daemon.hpp"

using namespace task_manager;

int main(int argc, char *argv[]) {
  try {
    Daemon daemon;
    daemon.run();
  } catch (const sdbus::Error &e) {
    syslog(LOG_ERROR, "An unhandled DBUS exception occurred: '%s'", e.what());
    return 1;
  }
  return 0;
}

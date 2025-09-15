#include "defines.hpp"

#include "Daemon.hpp"

using namespace task_manager;

int main(int argc, char *argv[]) {
  try {
    Daemon daemon;
    daemon.run();
  } catch (const sdbus::Error &e) {
    // Log sdbus errors
    return 1;
  }
  return 0;
}

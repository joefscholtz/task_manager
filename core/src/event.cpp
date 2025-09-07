#include "event.hpp"
#include <format>

namespace task_manager {

// DEFAULT_EVENT_ID =

std::ostream &operator<<(std::ostream &os, const Event &event) {
  os << "Id: " << event.get_id() << "\n"
     << "Name: " << event.get_name() << "\n"
     << "Start: " << std::format("{:%Y.%m.%d %H:%M}", event.get_start()) << "\n"
     << "End: " << std::format("{:%Y.%m.%d %H:%M}", event.get_end()) << "\n"
     << "Description: " << event.get_description() << "\n";
  return os;
}
} // namespace task_manager

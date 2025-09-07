#include "event.hpp"
#include <format>

namespace task_manager {

std::ostream &operator<<(std::ostream &os, const Event &event) {
  os << event.get_name() << "\n"
     << "Start: " << std::format("{:%Y.%m.%d %H:%M}", event.get_start()) << "\n"
     << "End: " << std::format("{:%Y.%m.%d %H:%M}", event.get_end()) << "\n"
     << "Description: " << event.get_description() << "\n";
  return os;
}
} // namespace task_manager

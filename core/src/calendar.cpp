#include "calendar.hpp"

namespace task_manager {
int Calendar::tick() {
  now = std::chrono::system_clock::now();
  update_events();
  return 0;
}

bool Calendar::update_events() {
  for (auto &event : this->_ongoing_events) {
    if (event.get_end() < now) {
      this->_past_events.emplace_back(std::move(event));
      continue;
    }
    if (event.get_end() > now) {
      this->_future_events.emplace_back(std::move(event));
      continue;
    }
  }
  return true;
}
} // namespace task_manager

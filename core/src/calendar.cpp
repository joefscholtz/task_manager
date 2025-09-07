#include "calendar.hpp"

namespace task_manager {
int Calendar::tick() {
  now = std::chrono::system_clock::now();
  update_events();
  return 0;
}

bool Calendar::update_events() {
  for (auto &event : this->_ongoing_events) {
    if (event->get_end() < now) {
      this->_past_events.emplace_back(std::move(event));
      continue;
    }
    if (event->get_start() > now) {
      this->_future_events.emplace_back(std::move(event));
      continue;
    }
  }
  return true;
}
bool Calendar::add_event(Event &event) {
  auto event_ptr = std::make_shared<Event>(event);
  auto start = event_ptr->get_start(); // Assuming event_ptr after the move
  auto end = event_ptr->get_end();

  this->_all_events.push_back(event_ptr);
  if (end < now) {
    this->_past_events.push_back(event_ptr);
  } else if (start > now) {
    this->_future_events.push_back(event_ptr);
  } else {
    this->_ongoing_events.push_back(event_ptr);
  }
  return true;
}

std::vector<std::shared_ptr<Event>> Calendar::get_events() {
  return this->_all_events;
}

std::ostream &operator<<(std::ostream &os, const Calendar &calendar) {
  for (const auto &event : calendar._all_events) {
    os << *event;
  }
  return os;
}
} // namespace task_manager

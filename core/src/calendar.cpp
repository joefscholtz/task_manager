#include "calendar.hpp"
#include "db.hpp"

namespace task_manager {

int Calendar::tick() {
  this->_now = std::chrono::system_clock::now();
  this->update_events();
  return 0;
}

bool Calendar::update_events() {
  this->_past_events.clear();
  this->_ongoing_events.clear();
  this->_future_events.clear();

  for (auto &event : this->_all_events) {
    if (event->get_end() < this->_now) {
      this->_past_events.push_back(event);
    } else if (event->get_start() > this->_now) {
      this->_future_events.push_back(event);
    } else {
      this->_ongoing_events.push_back(event);
    }
  }
  return true;
}

bool Calendar::add_event(Event &event) {
  auto event_ptr = std::make_shared<Event>(event);

  this->_all_events.push_back(event_ptr);

  if (event_ptr->get_end() < this->_now) {
    this->_past_events.push_back(event_ptr);
  } else if (event_ptr->get_start() > this->_now) {
    this->_future_events.push_back(event_ptr);
  } else {
    this->_ongoing_events.push_back(event_ptr);
  }
  return true;
}

void Calendar::load_events() {
  auto storage = this->get_storage();
  storage.sync_schema();
  auto db_events = storage.get_all<Event>();

  this->_all_events.clear();
  this->_all_events.reserve(db_events.size());

  for (auto &ev : db_events) {
    ev.update_members_from_db();
    this->add_event(ev);
  }

  this->update_events();
}

std::ostream &operator<<(std::ostream &os, const Calendar &calendar) {
  for (const auto &event : calendar._all_events) {
    os << *event;
  }
  return os;
}

} // namespace task_manager

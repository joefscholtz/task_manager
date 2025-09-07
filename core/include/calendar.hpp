#pragma once
#include "db.hpp"
#include "event.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

namespace task_manager {
using time_point = std::chrono::system_clock::time_point;

class Calendar {
public:
  using Storage = decltype(init_storage());

  Calendar(Storage &storage) : _storage(storage) { load_events_from_db(); }
  ~Calendar() = default;

  int tick();
  bool update_ongoing_events(
      bool clear = false,
      const time_point &time_p = std::chrono::system_clock::now());
  inline const std::vector<std::shared_ptr<Event>> get_events() const {
    return this->_all_events;
  }
  inline Storage &get_storage() { return this->_storage; }
  inline const Storage &get_storage() const { return this->_storage; }
  bool
  create_event(Event &event,
               const time_point &time_p = std::chrono::system_clock::now());
  friend std::ostream &operator<<(std::ostream &os, const Calendar &calendar);

private:
  bool load_event(Event &event,
                  const time_point &time_p = std::chrono::system_clock::now());
  void load_events_from_db();
  bool save_event_in_db(std::shared_ptr<Event> &event_ptr);
  bool update_event_in_db(std::shared_ptr<Event> &event_ptr);
  bool remove_event_from_db(std::shared_ptr<Event> &event_ptr);
  std::vector<std::shared_ptr<Event>> _past_events, _ongoing_events,
      _future_events, _all_events;
  Storage &_storage;
  time_point _now = std::chrono::system_clock::now();
};

} // namespace task_manager

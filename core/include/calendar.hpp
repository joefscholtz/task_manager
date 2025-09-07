#pragma once
#include "db.hpp"
#include "event.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

namespace task_manager {

class Calendar {
public:
  using Storage = decltype(init_storage());

  Calendar(Storage &storage) : _storage(storage) { load_events(); }
  ~Calendar() = default;

  int tick();
  bool update_events();
  bool add_event(Event &event);
  inline const std::vector<std::shared_ptr<Event>> get_events() const {
    return this->_all_events;
  }
  inline const Storage &get_storage() const { return this->_storage; }
  void load_events();
  friend std::ostream &operator<<(std::ostream &os, const Calendar &calendar);

private:
  std::vector<std::shared_ptr<Event>> _past_events, _ongoing_events,
      _future_events, _all_events;
  Storage &_storage;
  std::chrono::time_point<std::chrono::system_clock> _now =
      std::chrono::system_clock::now();
};

} // namespace task_manager

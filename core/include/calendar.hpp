#pragma once
#include "db.hpp"
#include "event.hpp"
#include "time.hpp"

namespace task_manager {
class Calendar {
public:
  Calendar(Storage &storage) : _storage(storage) { load_events(); }
  ~Calendar() = default;
  int tick();
  bool update_events();
  bool add_event(Event &event);
  std::vector<std::shared_ptr<Event>> get_events();
  friend std::ostream &operator<<(std::ostream &os, const Calendar &calendar);

private:
  void load_events();

  Storage &_storage;
  std::vector<std::shared_ptr<Event>> _past_events, _ongoing_events,
      _future_events, _all_events;
  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
};
} // namespace task_manager

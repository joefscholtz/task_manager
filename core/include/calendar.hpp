#include "defines.hpp"
#include "event.hpp"
#include "time.hpp"

namespace task_manager {
class Calendar {
public:
  Calendar() {}
  ~Calendar() = default;
  int tick();
  bool update_events();
  bool add_event(Event &event);
  std::vector<std::shared_ptr<Event>> get_events();
  friend std::ostream &operator<<(std::ostream &os, const Calendar &calendar);

private:
  std::vector<std::shared_ptr<Event>> _past_events, _ongoing_events,
      _future_events, _all_events;
  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
};
} // namespace task_manager

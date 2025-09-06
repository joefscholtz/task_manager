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

private:
  std::vector<Event> _past_events, _ongoing_events, _future_events;
  std::chrono::time_point<std::chrono::system_clock> now =
      std::chrono::system_clock::now();
};
} // namespace task_manager

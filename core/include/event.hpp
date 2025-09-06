#include "defines.hpp"
#include "time.hpp"

namespace task_manager {
using time_poit =
    std::chrono::time_point<std::chrono::system_clock> class Event {
public:
  Event(time_point &start, time_point &end);
  ~Event();
  inline time_point get_start() { return self._start; }
  void set_start(time_point &start);
  inline time_point get_end() { return self._end; }
  void set_end(time_point &end);

  inline time_point get_end() { return self._end; }
  void set_end(time_point &end);

  inline void start() { self._ongoing = true; }
  inline void pause() { self._ongoing = false; }

private:
  time_point _created, _last_changed;
  time_point _start, _end;
  std::string _description;
  bool _ongoing;
};
} // namespace task_manager

#include "defines.hpp"
#include "time.hpp"
#include <chrono>

namespace task_manager {
using time_point = std::chrono::system_clock::time_point;

class Event {
public:
  Event(const time_point &start, const time_point &end)
      : _start(start), _end(end) {}

  ~Event() = default;
  inline time_point get_start() { return this->_start; }
  inline void set_start(time_point &start) { this->_start = start; }
  inline time_point get_end() { return this->_end; }
  inline void set_end(time_point &end) { this->_end = end; }

  inline std::string get_description() { return this->_description; }
  inline void set_description(std::string &description) {
    this->_description = description;
  }

  inline void start() { this->_ongoing = true; }
  inline void pause() { this->_ongoing = false; }

private:
  time_point _created, _last_changed;
  time_point _start, _end;
  std::string _description;
  bool _ongoing;
};
} // namespace task_manager

#pragma once
#include "BaseApiEvent.hpp"
#include "defines.hpp"
#include "time.hpp"

namespace task_manager {

class Event {
public:
  Event(const std::string &name = "", const time_point &start = {},
        const time_point &end = {}, const uint32_t id = 0)
      : _name(name), _id(id) {
    set_start(start);
    set_end(end);
    _ongoing = false;
  }

  ~Event() = default;

  // This new method will synchronize your time_point members
  // after the ORM populates the _start_db and _end_db members.
  void update_members_from_db() {
    _start = time_point(std::chrono::microseconds(_start_db));
    _end = time_point(std::chrono::microseconds(_end_db));
  }

  inline const uint32_t &get_id() const { return this->_id; }
  inline void set_id(uint32_t id) { this->_id = id; }

  inline const std::optional<std::string> &get_iCalUID() const {
    return _iCalUID;
  }
  inline void set_iCalUID(const std::string &id) { _iCalUID = id; }

  inline const BaseApiEvent &get_external_api_event() const {
    return _external_api_event;
  }
  inline void set_external_api_event(const BaseApiEvent &api_event) {
    _external_api_event = api_event;
  }

  inline const std::string &get_name() const { return this->_name; }
  inline void set_name(const std::string &name) { this->_name = name; }

  inline const std::string &get_description() const {
    return this->_description;
  }
  inline void set_description(const std::string &description) {
    this->_description = description;
  }

  inline const time_point &get_start() const { return this->_start; }

  inline void set_start(const time_point &start) {
    this->_start = start;
    this->_start_db =
        std::chrono::time_point_cast<std::chrono::microseconds>(start)
            .time_since_epoch()
            .count();
  }

  inline const time_point &get_end() const { return this->_end; }

  inline void set_end(const time_point &end) {
    this->_end = end;
    this->_end_db = std::chrono::time_point_cast<std::chrono::microseconds>(end)
                        .time_since_epoch()
                        .count();
  }

  inline void start() { this->_ongoing = true; }
  inline void pause() { this->_ongoing = false; }

  friend std::ostream &operator<<(std::ostream &os, const Event &event);

  uint32_t _id;
  std::optional<std::string> _iCalUID;
  BaseApiEvent _external_api_event;
  time_point _start, _end;
  std::string _name;
  std::string _description;
  long long _start_db; // sqlite3 format for _start
  long long _end_db;   // sqlite3 format for _end
  bool _ongoing;
  std::optional<uint32_t> _account_id;
};
} // namespace task_manager

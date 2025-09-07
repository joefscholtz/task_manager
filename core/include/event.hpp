#pragma once
#include "defines.hpp"
#include "time.hpp"
#include <chrono>
#include <iostream>
#include <string>

namespace task_manager {
using time_point = std::chrono::system_clock::time_point;

class Event {
public:
  Event(const std::string &name = "", const time_point &start = {},
        const time_point &end = {})
      : _name(name), _id(0) {
    // Use the setters to initialize time members to ensure synchronization
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

  // --- Getters and Setters ---
  inline const int32_t &get_id() const { return this->_id; }
  inline void set_id(int32_t id) { this->_id = id; }

  inline const std::string &get_name() const { return this->_name; }
  inline void set_name(const std::string &name) { this->_name = name; }

  inline const std::string &get_description() const {
    return this->_description;
  }
  inline void set_description(const std::string &description) {
    this->_description = description;
  }

  // Getter for _start remains the same
  inline const time_point &get_start() const { return this->_start; }

  // Setter for _start now ALSO updates the database-friendly member
  inline void set_start(const time_point &start) {
    this->_start = start;
    this->_start_db =
        std::chrono::time_point_cast<std::chrono::microseconds>(start)
            .time_since_epoch()
            .count();
  }

  // Getter for _end remains the same
  inline const time_point &get_end() const { return this->_end; }

  // Setter for _end now ALSO updates the database-friendly member
  inline void set_end(const time_point &end) {
    this->_end = end;
    this->_end_db = std::chrono::time_point_cast<std::chrono::microseconds>(end)
                        .time_since_epoch()
                        .count();
  }

  inline void start() { this->_ongoing = true; }
  inline void pause() { this->_ongoing = false; }

  friend std::ostream &operator<<(std::ostream &os, const Event &event);

  // --- Member Variables ---
  // These are for your C++ logic. They will NOT be mapped to the database.
  time_point _start, _end;

  // These are the members that WILL be mapped to the database.
  // We make them public so sqlite_orm can access them directly.
  int32_t _id;
  std::string _name;
  std::string _description;
  long long _start_db; // Database-friendly format for _start
  long long _end_db;   // Database-friendly format for _end
  bool _ongoing;
};
} // namespace task_manager

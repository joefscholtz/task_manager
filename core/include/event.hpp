#pragma once
#include "defines.hpp"
#include "time.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <string>

namespace task_manager {
using time_point = std::chrono::system_clock::time_point;

class Event {
public:
  Event() = default; // needed by sqlite_orm
  Event(const std::string &name, const time_point &start, const time_point &end)
      : _name(name), _start(start), _end(end), _ongoing(false) {}

  ~Event() = default;

  inline const time_point &get_start() const { return this->_start; }
  inline void set_start(time_point &start) { this->_start = start; }

  inline const time_point &get_end() const { return this->_end; }
  inline void set_end(time_point &end) { this->_end = end; }

  inline const std::string &get_name() const { return this->_name; }
  inline void set_name(std::string &name) { this->_name = name; }

  inline const std::string &get_description() const {
    return this->_description;
  }
  inline void set_description(std::string &description) {
    this->_description = description;
  }

  inline void start() { this->_ongoing = true; }
  inline void pause() { this->_ongoing = false; }

  friend std::ostream &operator<<(std::ostream &os, const Event &event);

  // public for ORM access
  int32_t _id = 0;
  time_point _created, _last_changed;
  time_point _start, _end;
  std::string _name, _description;
  bool _ongoing = false;
};
} // namespace task_manager

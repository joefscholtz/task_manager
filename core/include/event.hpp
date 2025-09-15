#pragma once
#include "defines.hpp"
#include "time.hpp"

#include "BaseApiEvent.hpp"

namespace task_manager {

class Event {
public:
  // Since we manage a resource (shared_ptr), it's good practice to define
  // the "Rule of Five" special member functions, even if they are defaulted.
  Event(const std::string &name = "", const time_point &start = {},
        const time_point &end = {}, const uint32_t id = 0)
      : _name(name), _id(id) {
    set_start(start);
    set_end(end);
    _ongoing = false;
  }
  ~Event() = default;
  Event(const Event &other) = default;
  Event &operator=(const Event &other) = default;
  Event(Event &&other) noexcept = default;
  Event &operator=(Event &&other) noexcept = default;

  // This new method will synchronize your time_point members
  // after the ORM populates the _start_db and _end_db members.
  void update_members_from_db() {
    this->_start = time_point(std::chrono::microseconds(_start_db));
    this->_end = time_point(std::chrono::microseconds(_end_db));
  }

  inline const uint32_t &get_id() const { return this->_id; }
  inline void set_id(uint32_t id) { this->_id = id; }

  inline const std::optional<std::string> &get_iCalUID() const {
    return this->_iCalUID;
  }
  inline void set_iCalUID(const std::string &iCalUID) {
    this->_iCalUID = iCalUID;
  }

  inline void set_recurring_event_id(const std::string &recurring_event_id) {
    this->_recurring_event_id = recurring_event_id;
  }
  inline const std::string &get_recurring_event_id() const {
    return this->_recurring_event_id;
  }

  inline const std::optional<std::string> &get_etag() const {
    return this->_etag;
  }
  inline void set_etag(const std::string &etag) { this->_etag = etag; }

  const std::shared_ptr<BaseApiEvent> get_external_api_event_ptr() const {
    return this->_external_api_event_ptr;
  }

  inline void
  set_external_api_event_ptr(std::shared_ptr<BaseApiEvent> api_event_ptr) {
    this->_external_api_event_ptr = api_event_ptr;
    if (this->_external_api_event_ptr) {
      this->_external_account_type = api_event_ptr->get_account_type();
    }
  }
  inline void set_external_api_event_ptr(const BaseApiEvent &api_event) {
    set_external_api_event_ptr(std::make_shared<BaseApiEvent>(api_event));
  }

  inline const std::vector<std::shared_ptr<BaseApiEvent>> &
  get_external_api_recurring_events_ptr() const {
    return this->_external_api_recurring_events_ptr;
  }
  inline void set_external_api_recurring_events_ptr(
      const std::vector<std::shared_ptr<BaseApiEvent>>
          &external_api_recurring_events_ptr) {
    this->_external_api_recurring_events_ptr =
        external_api_recurring_events_ptr;
  }
  inline void push_back_external_api_recurring_event_ptr(
      const std::shared_ptr<BaseApiEvent> &external_api_recurring_event_ptr) {
    this->_external_api_recurring_events_ptr.push_back(
        external_api_recurring_event_ptr);
  }
  inline void clear_external_api_recurring_event_ptr() {
    this->_external_api_recurring_events_ptr.clear();
  }

  inline const bool &get_store_recurring_events() const {
    return this->_store_recurring_events;
  }
  inline void set_store_recurring_events(bool store_recurring_events) {
    this->_store_recurring_events = store_recurring_events;
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
  std::optional<std::string> _etag;
  std::string _recurring_event_id;
  std::shared_ptr<BaseApiEvent> _external_api_event_ptr;
  std::vector<std::shared_ptr<BaseApiEvent>> _external_api_recurring_events_ptr;
  bool _store_recurring_events = true;
  AccountType _external_account_type = AccountType::NOT_INHERITED;
  time_point _start, _end;
  std::string _name;
  std::string _description;
  long long _start_db; // sqlite3 format for _start
  long long _end_db;   // sqlite3 format for _end
  bool _ongoing;
  std::optional<uint32_t> _account_id;
};
} // namespace task_manager

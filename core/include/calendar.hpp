#pragma once
#include "GCalApiEvent.hpp"
#include "GCalApiEventsList.hpp"
#include "account.hpp"
#include "db.hpp"
#include "defines.hpp"
#include "event.hpp"
#include "gcal_api.hpp"
#include "time.hpp"

namespace task_manager {

class Calendar {
public:
  using Storage = decltype(init_storage());

  Calendar() : _storage(init_storage()) { load_events_from_db(); }
  ~Calendar() = default;

  int tick();
  bool update_ongoing_events(
      bool clear = false,
      const time_point &time_p = std::chrono::system_clock::now());
  inline const std::vector<std::shared_ptr<Event>> get_events() const {
    return this->_all_events;
  }
  inline Storage &get_storage() { return this->_storage; }
  inline const Storage &get_storage() const { return this->_storage; }
  bool
  create_event(Event &event,
               const time_point &time_p = std::chrono::system_clock::now());
  bool update_event_by_id(uint32_t id, const std::string &name,
                          const std::string &desc);
  bool remove_event_by_id(u_int32_t id);

  friend std::ostream &operator<<(std::ostream &os, const Calendar &calendar);

  bool link_google_account(
      std::string client_secret_path = std::string(".env/client_secret.json"));
  void sync_external_events();

private:
  bool load_event(Event &event,
                  const time_point &time_p = std::chrono::system_clock::now());
  void load_events_from_db();
  void load_events();
  bool save_event_in_db(std::shared_ptr<Event> &event_ptr);
  bool save_account_in_db(std::shared_ptr<Account> &account_ptr);
  bool update_event_in_db(std::shared_ptr<Event> &event_ptr);
  bool remove_event_from_db(std::shared_ptr<Event> &event_ptr);
  std::vector<std::shared_ptr<Event>> _past_events, _ongoing_events,
      _future_events, _all_events;
  Storage _storage;
  time_point _now = std::chrono::system_clock::now();
  std::vector<std::shared_ptr<Account>> _accounts;
  std::unique_ptr<GoogleCalendarAPI> _gcal_api;

  // std::unordered_map<std::string, std::unique_ptr<GoogleCalendarAPI>>
  //     _gcal_apis;
};

} // namespace task_manager

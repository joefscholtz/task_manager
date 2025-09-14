#include "calendar.hpp"
#include "db.hpp"
#include <memory>

namespace task_manager {

int Calendar::tick() {
  this->_now = std::chrono::system_clock::now();
  this->update_ongoing_events(false, this->_now);
  return 0;
}

bool Calendar::update_ongoing_events(bool clear, const time_point &time_p) {
  try {
    auto reclassify = [&](auto &src, auto &dst, auto pred) {
      for (auto it = src.begin(); it != src.end();) {
        if (pred(*it)) {
          dst.push_back(std::move(*it));
          it = src.erase(it);
        } else {
          ++it;
        }
      }
    };

    if (clear) {
      // full rebuild
      this->_past_events.clear();
      this->_ongoing_events.clear();
      this->_future_events.clear();

      this->_past_events.reserve(this->_all_events.size());
      this->_ongoing_events.reserve(
          5); // arbitrary number, only a few will be ongoing at a given time
      this->_future_events.reserve(this->_all_events.size());

      for (auto &event_ptr : this->_all_events) {
        if (event_ptr->get_end() < time_p) {
          this->_past_events.push_back(event_ptr);
        } else if (event_ptr->get_start() > time_p) {
          this->_future_events.push_back(event_ptr);
        } else {
          this->_ongoing_events.push_back(event_ptr);
        }
      }
    } else {
      // incremental update
      reclassify(this->_past_events, this->_future_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() > time_p;
                 });

      reclassify(this->_past_events, this->_ongoing_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() <= time_p &&
                          event_ptr->get_end() >= time_p;
                 });

      reclassify(
          this->_ongoing_events, this->_past_events,
          [&](const auto &event_ptr) { return event_ptr->get_end() < time_p; });

      reclassify(this->_ongoing_events, this->_future_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() > time_p;
                 });

      reclassify(
          this->_future_events, this->_past_events,
          [&](const auto &event_ptr) { return event_ptr->get_end() < time_p; });

      reclassify(this->_future_events, this->_ongoing_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() <= time_p &&
                          event_ptr->get_end() >= time_p;
                 });
    }
  } catch (const std::exception &e) {
    std::cerr << "Error updating events: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Unknown error updating events" << std::endl;
    return false;
  }

  return true;
}

bool Calendar::load_event(Event &event, const time_point &time_p) {
  auto event_ptr = std::make_shared<Event>(event);

  this->_all_events.push_back(event_ptr);

  if (event_ptr->get_end() < time_p) {
    this->_past_events.push_back(event_ptr);
  } else if (event_ptr->get_start() > time_p) {
    this->_future_events.push_back(event_ptr);
  } else {
    this->_ongoing_events.push_back(event_ptr);
  }
  return true;
}

void Calendar::load_events_from_db() {
  auto load_time_p = std::chrono::system_clock::now();
  auto storage = this->get_storage();
  // TODO: use log library
  // std::cout << "Got Storage" << std::endl;
  storage.sync_schema();
  // TODO: use log library
  // std::cout << "Schema Synced" << std::endl;
  auto db_events = storage.get_all<Event>();
  // TODO: use log library
  // std::cout << "Got db events" << std::endl;

  // TODO: use log library
  // std::cout << "Stored Events: " << std::endl;
  // for (const auto &event : db_events) {
  //   std::cout << event;
  // }

  this->_all_events.clear();
  this->_all_events.reserve(db_events.size());

  for (auto &ev : db_events) {
    ev.update_members_from_db();
    this->load_event(ev, load_time_p);
  }
}
void Calendar::load_accounts_from_db() {
  auto storage = this->get_storage();
  storage.sync_schema();
  if (storage.count<Account>() == 0) {

    std::shared_ptr<Account> local_account = std::make_shared<Account>("local");
    this->save_account_in_db(local_account);
  }

  auto db_accounts = storage.get_all<Account>();
  this->_accounts.reserve(db_accounts.size());

  for (auto db_account : db_accounts) {
    _accounts.push_back(std::make_shared<Account>(db_account));
  }
}
void Calendar::load_events() {
  this->load_events_from_db();
  this->load_accounts_from_db();
  this->sync_external_events();
}

bool Calendar::save_event_in_db(std::shared_ptr<Event> &event_ptr) {
  try {
    _storage.transaction([&]() {
      // TODO: debug
      // std::cout << "inserting: \n" << *event_ptr << std::endl;
      auto updated_id = _storage.insert(*event_ptr);
      // TODO: debug
      // std::cout << "updated_id: " << updated_id << std::endl;
      event_ptr->set_id(static_cast<uint32_t>(updated_id));
      return true;
    });
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving event: " << e.what() << std::endl;
    return false;
  }
}

bool Calendar::save_account_in_db(std::shared_ptr<Account> &account_ptr) {
  try {
    _storage.transaction([&]() {
      auto updated_id = _storage.insert(*account_ptr);
      // TODO: debug
      // std::cout << "updated_id: " << updated_id << std::endl;
      account_ptr->set_id(static_cast<uint32_t>(updated_id));
      return true;
    });
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving account: " << e.what() << std::endl;
    return false;
  }
}

bool Calendar::create_event(Event &event, const time_point &time_p) {
  auto event_ptr = std::make_shared<Event>(event);
  if (!this->save_event_in_db(event_ptr))
    return false;

  this->_all_events.push_back(event_ptr);

  if (event_ptr->get_end() < time_p) {
    this->_past_events.push_back(event_ptr);
  } else if (event_ptr->get_start() > time_p) {
    this->_future_events.push_back(event_ptr);
  } else {
    this->_ongoing_events.push_back(event_ptr);
  }
  return true;
}

bool Calendar::update_event_in_db(std::shared_ptr<Event> &event_ptr) {
  try {
    _storage.transaction([&]() {
      _storage.update(*event_ptr);
      return true;
    });
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error updating event: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Unknown error updating event" << std::endl;
    return false;
  }
}

bool Calendar::update_event_by_id(uint32_t id, const std::string &name,
                                  const std::string &desc) {
  auto it = std::find_if(_all_events.begin(), _all_events.end(),
                         [id](const auto &e) { return e->get_id() == id; });
  if (it == _all_events.end())
    return false;

  auto event_ptr = *it;
  if (!name.empty())
    event_ptr->set_name(name);
  if (!desc.empty())
    event_ptr->set_description(desc);
  return update_event_in_db(event_ptr);
}

bool Calendar::remove_event_from_db(std::shared_ptr<Event> &event_ptr) {
  try {
    _storage.transaction([&]() {
      _storage.remove<Event>(event_ptr->get_id());
      return true;
    });

    auto remove_from_vector = [&](auto &vec) {
      vec.erase(std::remove(vec.begin(), vec.end(), event_ptr), vec.end());
    };

    remove_from_vector(this->_all_events);
    remove_from_vector(this->_past_events);
    remove_from_vector(this->_ongoing_events);
    remove_from_vector(this->_future_events);

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error removing event: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Unknown error removing event" << std::endl;
    return false;
  }
}

bool Calendar::remove_event_by_id(uint32_t id) {
  auto it = std::find_if(
      this->_all_events.begin(), this->_all_events.end(),
      [&](const auto &event_ptr) { return event_ptr->get_id() == id; });

  if (it != this->_all_events.end()) {
    auto event_ptr = *it;
    if (this->remove_event_from_db(event_ptr)) {
      std::cout << "Removed event with id: " << event_ptr->get_id()
                << std::endl;
      return true;
    } else {
      std::cerr << "Failed to remove event from DB\n";
      return false;
    }
  } else {
    std::cout << "Event not found" << std::endl;
    return false;
  }
}

std::ostream &operator<<(std::ostream &os, const Calendar &calendar) {
  for (size_t i = 0; i < calendar._all_events.size(); ++i) {
    os << *calendar._all_events[i];
    if (i < calendar._all_events.size() - 1) {
      os << "--\n";
    }
  }
  return os;
}

bool Calendar::link_google_account(std::string client_secret_path) {
  std::string email;
  this->_gcal_api = std::make_unique<GoogleCalendarAPI>(client_secret_path);
  if (!this->_gcal_api) {
    std::cout << "Failed to create GoogleCalendarAPI object" << std::endl;
    return false;
  }
  if (!this->_gcal_api->authenticate()) {
    std::cout << "Failed to authenticate" << std::endl;
    return false;
  } else {
    auto email_opt = this->_gcal_api->get_user_email();
    if (email_opt) {
      email = *email_opt;
      std::cout << "Successfully authenticated as: " << email << std::endl;
    } else {
      std::cerr << "Could not retrieve email for the new account." << std::endl;
      return false;
    }
    std::shared_ptr<Account> gcal_account_ptr =
        std::make_shared<Account>(email);
    gcal_account_ptr->set_account_type(AccountType::GCAL);
    gcal_account_ptr->set_user_info(this->_gcal_api->get_user_info());
    gcal_account_ptr->set_refresh_token(this->_gcal_api->get_refresh_token());
    this->_gcal_api->clear_account();

    this->_accounts.push_back(gcal_account_ptr);
    return true;
  }
}

bool Calendar::sync_external_events() {
  bool sync_success = true;
  std::vector<std::shared_ptr<BaseApiEvent>> all_external_api_event_ptrs;

  // Iterate through all linked accounts to fetch their events
  for (const auto &account : this->_accounts) {
    if (account->get_account_type() == AccountType::GCAL) {
      if (!this->_gcal_api) {
        std::cerr << "Error: Google Calendar API is not initialized."
                  << std::endl;
        sync_success = false;
        continue;
      }

      std::cout << "Syncing with Google Calendar for account: "
                << account->get_email() << "..." << std::endl;

      std::optional<std::vector<GCalApiEvent>> fetched_events_opt =
          this->_gcal_api->list_events(500, account->get_refresh_token());
      account->set_refresh_token(this->_gcal_api->get_refresh_token());
      this->_gcal_api->clear_account();

      if (!fetched_events_opt) {
        std::cerr << "Error: Could not sync account: " << account->get_email()
                  << std::endl;
        sync_success = false;
        continue;
      }

      for (auto &event : fetched_events_opt.value()) {
        all_external_api_event_ptrs.push_back(
            std::make_shared<GCalApiEvent>(std::move(event)));
      }
    } else if (account->get_account_type() == AccountType::UNKNOWN) {
      // TODO: debug
      std::cout << "Account has unknown account_type" << std::endl;

    } else {
      // TODO: debug
      std::cout << "Account has wrong account_type" << std::endl;
    }
  }

  std::cout << "\nTotal ApiEvents fetched: "
            << all_external_api_event_ptrs.size() << std::endl;

  for (const auto &api_event_ptr : all_external_api_event_ptrs) {
    if (api_event_ptr->get_account_type() == AccountType::UNKNOWN) {
      // TODO: debug
      std::cout << "ApiEvent has unknown account_type" << std::endl;
      continue;
    } else if (api_event_ptr->get_account_type() != AccountType::GCAL) {
      // TODO: debug
      std::cout << "ApiEvent has wrong account_type" << std::endl;
      continue;
    }

    const auto gcal_api_event_ptr =
        std::dynamic_pointer_cast<const GCalApiEvent>(api_event_ptr);
    if (!gcal_api_event_ptr) {
      std::cerr << "Error: ApiEvent pointer type is a GCalApiEvent pointer but "
                   "dynamic_pointer_cast failed."
                << std::endl;
      sync_success = false;
      continue;
    }

    const GCalApiEvent &gcal_api_event = *gcal_api_event_ptr;

    bool exists = false;
    for (const auto &existing_event : _all_events) {
      if (existing_event->get_iCalUID() == gcal_api_event.iCalUID) {
        auto external_api_event_base_ptr =
            existing_event->get_external_api_event_ptr();

        if (!external_api_event_base_ptr) {
          // TODO: should I fill it then??
          std::cerr
              << "Error: external_api_event_base_ptr is null: existing_event "
                 "doesnt have an external_api_event. Should it be filled?"
              << std::endl;
          sync_success = false;
          continue;
        }

        const auto existing_gcal_api_event_ptr =
            std::dynamic_pointer_cast<const GCalApiEvent>(
                external_api_event_base_ptr);

        if (!existing_gcal_api_event_ptr) {
          std::cerr << "Error: existing_api_event type is GCalApiEvent but "
                       "dynamic_pointer_cast failed."
                    << std::endl;
          sync_success = false;
          continue;
        }

        const GCalApiEvent &existing_gcal_api_event =
            *existing_gcal_api_event_ptr;

        if (gcal_api_event.etag != existing_gcal_api_event.etag) {
          // TODO: update_event(existing_event, gcal_api_event);
          // this->update_event_by_id(existing_event->get_id());
          std::cerr << "ApiEvent should be updated!" << std::endl;
        } else {
          // TODO: recurring_events
          std::cerr << "Recurring event found" << std::endl;
        }
        exists = true;
        break;
      }
    }

    if (!exists) {
      Event new_event;
      new_event.set_name(gcal_api_event.summary);
      new_event.set_start(
          this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.start));
      new_event.set_end(
          this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.end));
      new_event.set_iCalUID(gcal_api_event.iCalUID);
      new_event.set_external_api_event_ptr(api_event_ptr);

      this->create_event(new_event);
      std::cout << "  + Added remote event: " << new_event.get_name()
                << std::endl;
    }
  }

  sync_success &= update_ongoing_events();

  if (sync_success) {
    std::cout << "\nSync complete." << std::endl;
  } else {
    std::cout << "\nSync completed with one or more errors." << std::endl;
  }

  return sync_success;
}

} // namespace task_manager

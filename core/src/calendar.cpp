#include "calendar.hpp"
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
bool Calendar::load_account(Account &account) {
  account.update_members_from_db();
  auto account_ptr = std::make_shared<Account>(account);

  this->_accounts.push_back(account_ptr);
  return true;
}

bool Calendar::load_event(Event &event, const time_point &time_p) {
  event.update_members_from_db();
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
  // TODO: debug
  std::cout << "load_events_from_db" << std::endl;

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
  // TODO: debug
  std::cout << "load_accounts_from_db" << std::endl;
  auto storage = this->get_storage();
  storage.sync_schema();
  // TODO: debug
  std::cout << "synced schema in load_accounts_from_db" << std::endl;
  if (storage.count<Account>() == 0) {
    // TODO: debug
    std::cout << "storage has no local account, creating.." << std::endl;
    std::shared_ptr<Account> local_account_ptr =
        std::make_shared<Account>("local");
    local_account_ptr->set_account_type(AccountType::LOCAL);
    if (this->save_account_in_db(local_account_ptr)) {
      // TODO: debug
      std::cout << "local account created" << std::endl;
    } else {
      // TODO: debug
      std::cout << "error saving local account in db" << std::endl;
    }
  } else {
    // TODO: debug
    std::cout << "storage already had accounts" << std::endl;
  }

  // TODO: debug
  std::cout << "updating calendar accounts from db" << std::endl;
  auto db_accounts = storage.get_all<Account>();
  this->_accounts.reserve(db_accounts.size());
  for (auto db_account : db_accounts) {
    db_account.update_members_from_db();
    this->load_account(db_account);
  }
}
void Calendar::load_events() {
  // TODO: debug
  std::cout << "load_events" << std::endl;
  this->load_accounts_from_db();
  this->load_events_from_db();
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

bool Calendar::update_account_in_db(std::shared_ptr<Account> &account_ptr) {
  try {
    _storage.transaction([&]() {
      _storage.update(*account_ptr);
      return true;
    });
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error updating account: " << e.what() << std::endl;
    return false;
  } catch (...) {
    std::cerr << "Unknown error updating account" << std::endl;
    return false;
  }
}

bool Calendar::create_event(std::shared_ptr<Event> &event_ptr,
                            const time_point &time_p) {
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
bool Calendar::create_event(Event &event, const time_point &time_p) {
  auto event_ptr = std::make_shared<Event>(event);
  return this->create_event(event_ptr, time_p);
}

bool Calendar::update_event_in_db(const std::shared_ptr<Event> &event_ptr) {
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

bool Calendar::update_event_by_id(const uint32_t id, const std::string &name,
                                  const std::string &desc) {
  auto it = std::find_if(this->_all_events.begin(), this->_all_events.end(),
                         [id](const auto &e) { return e->get_id() == id; });
  if (it == this->_all_events.end())
    return false;

  const auto event_ptr = *it;
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

bool Calendar::link_google_account() {
  std::string email;
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
    this->save_account_in_db(gcal_account_ptr);

    return true;
  }
}
// TODO: Maybe move to db
bool Calendar::nuke_n_pave(std::shared_ptr<Account> &account) {
  std::cout << "  - Clearing old recurring event instances..." << std::endl;
  this->_storage.remove_all<Event>(
      where(c(&Event::_account_id) == account->get_id() and
            is_not_null(&Event::_recurring_event_id)));

  return true;
}

// TODO: return account_status
bool Calendar::check_account_api(const std::shared_ptr<Account> &account) {
  auto account_type = account->get_account_type();
  if (account_type == AccountType::LOCAL) {
    return false; // nothing to do
  } else if (account_type != AccountType::GCAL) {
    std::cerr << "Api not implemented for AccountType: "
              << account_type_to_string(account_type) << std::endl;
    return false;
  } else {
    if (!this->_gcal_api) {
      std::cerr << "Error: Google Calendar API is not initialized."
                << std::endl;
      return false;
    }
    std::cout << "Syncing with Google Calendar for account: "
              << account->get_email() << "..." << std::endl;
  }
  return true;
}

bool Calendar::sync_external_events() {
  // TODO: debug
  std::cout << "sync_external_events" << std::endl;
  bool sync_success = true;
  std::vector<std::shared_ptr<BaseApiEvent>> all_external_api_events_ptr;

  // Fetch remote events and wipe old recurring instances
  for (auto &account_ptr : this->_accounts) {

    if (!this->check_account_api(account_ptr)) {
      sync_success = false;
      continue;
    }

    // --- "Nuke and Pave" for recurring events ---
    // Before fetching, delete all existing recurring instances for this
    // account. This prevents duplicates when a whole series is moved or
    // changed.
    this->nuke_n_pave(account_ptr);

    auto fetched_events_opt =
        this->_gcal_api->list_events(500, account_ptr->get_refresh_token());
    account_ptr->set_refresh_token(this->_gcal_api->get_refresh_token());
    this->update_account_in_db(account_ptr);
    this->_gcal_api->clear_account();

    if (!fetched_events_opt) {
      std::cerr << "Error: Could not sync account: " << account_ptr->get_email()
                << std::endl;
      sync_success = false;
      continue;
    }

    for (auto &event : fetched_events_opt.value()) {
      all_external_api_events_ptr.push_back(
          std::make_shared<GCalApiEvent>(std::move(event)));
    }
  }

  std::cout << "\nTotal ApiEvents fetched: "
            << all_external_api_events_ptr.size() << std::endl;

  std::map<std::string, std::shared_ptr<Event>> single_events_map;
  std::map<std::string, std::shared_ptr<Event>> recurring_series_map;

  for (const auto &event_ptr : this->_all_events) {
    if (!event_ptr->get_recurring_event_id().empty()) {
      // This event is the primary for a recurring series
      recurring_series_map[event_ptr->get_recurring_event_id()] = event_ptr;
      event_ptr->clear_external_api_recurring_event_ptr();
    } else if (auto ical_uid = event_ptr->get_iCalUID()) {
      // This is a single, one-off event
      single_events_map[*ical_uid] = event_ptr;
    }
  }

  // process fetched events (create or update)
  for (const auto &api_event_ptr : all_external_api_events_ptr) {
    auto account_type = api_event_ptr->get_account_type();
    if (account_type == AccountType::LOCAL) {
      std::cerr << "AccountType: " << account_type_to_string(account_type)
                << " skiping" << std::endl;
      continue;
    } else if (account_type != AccountType::GCAL) {
      std::cerr << "create/update not implemented for AccountType"
                << account_type_to_string(account_type) << " skiping"
                << std::endl;
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

    if (!gcal_api_event.recurringEventId.empty()) {
      auto series_it =
          recurring_series_map.find(gcal_api_event.recurringEventId);

      if (series_it != recurring_series_map.end()) {
        // Recurring event in db found, add recurring instance to
        // _external_api_recurring_event_ptr vector

        // Primary local Event for this series.
        std::shared_ptr<Event> primary_event_ptr = series_it->second;

        // Add this new instance to the primary event's recurring list if
        // enabled
        if (primary_event_ptr->get_store_recurring_events()) {
          primary_event_ptr->push_back_external_api_recurring_event_ptr(
              api_event_ptr);
        }
        // update primary instance
        // TODO: fetch primary event from to get the actual primary event
        // updated
        primary_event_ptr->set_name(gcal_api_event.summary);
        primary_event_ptr->set_start(this->_gcal_api->parse_gcal_event_datetime(
            gcal_api_event.start)); // For a recurring event, start is the
                                    // start time of the first instance.
        primary_event_ptr->set_end(this->_gcal_api->parse_gcal_event_datetime(
            gcal_api_event.end)); // For a recurring event, end is the end
                                  // time of the first instance.
        primary_event_ptr->set_etag(gcal_api_event.etag);
        primary_event_ptr->set_recurring_event_id(
            gcal_api_event.recurringEventId);

        sync_success &= this->update_event_in_db(primary_event_ptr);

      } else {
        // FIRST instance seen for a new recurring series.
        // create a new primary Event for it.
        Event new_primary_event;
        new_primary_event.set_name(gcal_api_event.summary);
        new_primary_event.set_start(
            this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.start));
        new_primary_event.set_end(
            this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.end));
        new_primary_event.set_iCalUID(gcal_api_event.iCalUID);
        new_primary_event.set_recurring_event_id(
            gcal_api_event.recurringEventId);
        new_primary_event.set_external_api_event_ptr(api_event_ptr);

        std::shared_ptr<Event> new_primary_event_ptr =
            std::make_shared<Event>(new_primary_event);
        if (this->create_event(new_primary_event_ptr)) {
          recurring_series_map[gcal_api_event.recurringEventId] =
              new_primary_event_ptr;
        } else {
          sync_success = false;
        }
      }
    } else {
      // Single events
      auto event_it = single_events_map.find(gcal_api_event.iCalUID);
      if (event_it != single_events_map.end()) {
        std::shared_ptr<Event> existing_event_ptr = event_it->second;
        auto existing_api_ptr =
            existing_event_ptr->get_external_api_event_ptr();

        bool needs_update = true;
        if (existing_api_ptr) {
          auto existing_gcal_api_ptr =
              std::dynamic_pointer_cast<const GCalApiEvent>(existing_api_ptr);
          if (existing_gcal_api_ptr) {
            if (existing_gcal_api_ptr->etag == gcal_api_event.etag) {
              needs_update = false; // ETags match, no update needed.
            }
          }
        }

        if (needs_update) {
          existing_event_ptr->set_name(gcal_api_event.summary);
          existing_event_ptr->set_start(
              this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.start));
          existing_event_ptr->set_end(
              this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.end));
          existing_event_ptr->set_etag(gcal_api_event.etag);
          existing_event_ptr->set_recurring_event_id(
              gcal_api_event.recurringEventId);
          existing_event_ptr->set_external_api_event_ptr(api_event_ptr);

          sync_success &= this->update_event_in_db(existing_event_ptr);
        }
      } else {
        Event new_event;
        new_event.set_name(gcal_api_event.summary);
        new_event.set_start(
            this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.start));
        new_event.set_end(
            this->_gcal_api->parse_gcal_event_datetime(gcal_api_event.end));
        new_event.set_iCalUID(gcal_api_event.iCalUID);
        new_event.set_etag(gcal_api_event.etag);
        new_event.set_external_api_event_ptr(api_event_ptr);
        new_event.set_recurring_event_id(gcal_api_event.recurringEventId);

        sync_success &= this->create_event(new_event);

        std::cout << "  + Added remote event: " << new_event.get_name()
                  << std::endl;
      }
    }
  }

  // WARN: This simplified algorithm does not handle deletions of single
  // events. A full diffing algorithm would be needed for that.
  sync_success &= update_ongoing_events();
  std::cout << "\nSync complete." << std::endl;
  return sync_success;
}

} // namespace task_manager

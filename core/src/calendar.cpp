#include "calendar.hpp"
#include "db.hpp"

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
  storage.sync_schema();
  auto db_events = storage.get_all<Event>();

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

bool Calendar::save_event_in_db(std::shared_ptr<Event> &event_ptr) {
  try {
    _storage.transaction([&]() {
      auto updated_id = _storage.insert(*event_ptr);
      event_ptr->set_id(static_cast<uint32_t>(updated_id));
      return true;
    });
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving event: " << e.what() << std::endl;
    return false;
  }
}

bool Calendar::create_event(Event &event, const time_point &time_p) {
  auto event_ptr = std::make_shared<Event>(event);
  this->save_event_in_db(event_ptr);

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

bool Calendar::update_event_in_db(std::shared_ptr<Event> &event_ptr) {}

bool Calendar::remove_event_from_db(std::shared_ptr<Event> &event_ptr) {}

std::ostream &operator<<(std::ostream &os, const Calendar &calendar) {
  for (size_t i = 0; i < calendar._all_events.size(); ++i) {
    os << *calendar._all_events[i];
    if (i < calendar._all_events.size() - 1) {
      os << "--\n";
    }
  }
  return os;
}

} // namespace task_manager

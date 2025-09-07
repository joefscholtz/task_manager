#include "calendar.hpp"
#include "db.hpp"

namespace task_manager {

int Calendar::tick() {
  this->_now = std::chrono::system_clock::now();
  this->update_events();
  return 0;
}

bool Calendar::update_events(bool clear) {
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
        if (event_ptr->get_end() < this->_now) {
          this->_past_events.push_back(event_ptr);
        } else if (event_ptr->get_start() > this->_now) {
          this->_future_events.push_back(event_ptr);
        } else {
          this->_ongoing_events.push_back(event_ptr);
        }
      }
    } else {
      // incremental update
      reclassify(this->_past_events, this->_future_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() > this->_now;
                 });

      reclassify(this->_past_events, this->_ongoing_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() <= this->_now &&
                          event_ptr->get_end() >= this->_now;
                 });

      reclassify(this->_ongoing_events, this->_past_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_end() < this->_now;
                 });

      reclassify(this->_ongoing_events, this->_future_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() > this->_now;
                 });

      reclassify(this->_future_events, this->_past_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_end() < this->_now;
                 });

      reclassify(this->_future_events, this->_ongoing_events,
                 [&](const auto &event_ptr) {
                   return event_ptr->get_start() <= this->_now &&
                          event_ptr->get_end() >= this->_now;
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

bool Calendar::add_event(Event &event) {
  auto event_ptr = std::make_shared<Event>(event);

  this->_all_events.push_back(event_ptr);

  if (event_ptr->get_end() < this->_now) {
    this->_past_events.push_back(event_ptr);
  } else if (event_ptr->get_start() > this->_now) {
    this->_future_events.push_back(event_ptr);
  } else {
    this->_ongoing_events.push_back(event_ptr);
  }
  return true;
}

void Calendar::load_events() {
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
    this->add_event(ev);
  }

  this->update_events();
}

bool Calendar::save_events() {
  try {
    _storage.transaction([&]() {
      for (auto &event_ptr : this->_all_events) {
        // if (event_ptr->get_id() == DEFAULT_EVENT_ID)
        auto updated_id = _storage.insert(*event_ptr);
        event_ptr->set_id(updated_id);
      }
      return true;
    });
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving events: " << e.what() << std::endl;
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

} // namespace task_manager

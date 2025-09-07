#include "db.hpp"
#include "calendar.hpp"
#include "event.hpp"
#include <sqlite_orm/sqlite_orm.h>

namespace task_manager {

inline auto init_storage(const std::string &path) {
  using namespace sqlite_orm;
  return make_storage(
      path,
      make_table("events",
                 make_column("id", &Event::_id, autoincrement(), primary_key()),
                 make_column("name", &Event::_name),
                 make_column("description", &Event::_description),
                 make_column("start", &Event::_start),
                 make_column("end", &Event::_end)));
}

// CRUD Examples

void save_event(Event &ev, decltype(init_storage("")) &storage) {
  storage.sync_schema();
  storage.replace(ev);
}

std::vector<Event> get_all_events(decltype(init_storage("")) &storage) {
  storage.sync_schema();
  return storage.get_all<Event>();
}

void update_event(Event &ev, decltype(init_storage("")) &storage) {
  storage.update(ev);
}

void delete_event(Event &ev, decltype(init_storage("")) &storage) {
  storage.remove(ev);
}

} // namespace task_manager

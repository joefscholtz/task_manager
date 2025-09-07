#pragma once
#include "defines.hpp"
#include "event.hpp"
#include <sqlite_orm/sqlite_orm.h>

namespace task_manager {

inline auto init_storage(const std::string &db_path) {
  using namespace sqlite_orm;

  return make_storage(
      db_path,
      make_table("Event",
                 make_column("id", &Event::_id, autoincrement(), primary_key()),
                 make_column("name", &Event::_name),
                 make_column("description", &Event::_description),
                 make_column("start", &Event::_start),
                 make_column("end", &Event::_end),
                 make_column("ongoing", &Event::_ongoing)));
}

using Storage = decltype(init_storage(""));
} // namespace task_manager

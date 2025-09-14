#pragma once
#include "defines.hpp"

#include "account.hpp"
#include "event.hpp"
#include "sqlite_orm/sqlite_orm.h"

namespace task_manager {

using namespace sqlite_orm;

inline std::string get_user_db_path() {
  const char *xdg_data = std::getenv("XDG_DATA_HOME");
  std::filesystem::path base =
      xdg_data ? xdg_data
               : std::filesystem::path(std::getenv("HOME")) / ".local/share";
  std::filesystem::create_directories(base / "task_manager");
  return (base / "task_manager" / "task_manager.sqlite3").string();
}

inline auto init_storage(const std::string &db_path = get_user_db_path()) {
  return make_storage(
      db_path,
      make_table("events",
                 make_column("id", &Event::_id, primary_key().autoincrement()),
                 make_column("iCalUID", &Event::_iCalUID, unique()),
                 make_column("name", &Event::_name),
                 make_column("description", &Event::_description),
                 make_column("start", &Event::_start_db),
                 make_column("end", &Event::_end_db),
                 make_column("ongoing", &Event::_ongoing),
                 make_column("account_id", &Event::_account_id),
                 foreign_key(&Event::_account_id).references(&Account::_id)),
      make_table(
          "accounts",
          make_column("id", &Account::_id, primary_key().autoincrement()),
          make_column("email", &Account::_email, unique()),
          make_column("account_type", &Account::_account_type_db),
          make_column("refresh_token", &Account::_refresh_token)));
}

} // namespace task_manager

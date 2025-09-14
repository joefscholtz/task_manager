#pragma once

#include "defines.hpp"
#include "sqlite_orm/sqlite_orm.h"

namespace sqlite_orm {

// Tell the ORM how to store the enum in the database.
// We are mapping AccountType to an INTEGER column.
template <>
struct type_printer<task_manager::AccountType> : public integer_printer {};

// Tell the ORM how to read the enum from the database.
// This converts an integer from a row back into an AccountType.
template <> struct row_extractor<task_manager::AccountType> {
  task_manager::AccountType extract(const char *row_value) {
    return static_cast<task_manager::AccountType>(std::atoi(row_value));
  }

  task_manager::AccountType extract(int row_value) {
    return static_cast<task_manager::AccountType>(row_value);
  }
};

// Tell the ORM how to bind the enum to a SQL statement.
// This converts an AccountType variable into an integer for queries.
template <> struct statement_binder<task_manager::AccountType> {
  int bind(sqlite3_stmt *stmt, int index,
           const task_manager::AccountType &value) {
    return sqlite3_bind_int(stmt, index, static_cast<int>(value));
  }
};

} // namespace sqlite_orm

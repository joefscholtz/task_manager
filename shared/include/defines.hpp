#pragma once
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>

namespace task_manager {

enum class AccountType { GCAL, UNKNOWN };

inline const std::string account_type_to_string(AccountType type) {
  switch (type) {
  case AccountType::GCAL:
    return "Google Calendar";
  case AccountType::UNKNOWN:
    return "Unknown";
  default:
    return "Invalid Account Type";
  }
}

} // namespace task_manager

#pragma once
#include "nlohmann/json.hpp"
#include <cpr/cpr.h>
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

using json = nlohmann::json;

// NOT_INHERITED: when its a class member: should've be inherited from another
// member, source_member, of this class even if the source_member's AccountType
// is UNKNOWN, use to prevent access before initialization
//
// UNKNOWN: when its a class member: the class instance is yet to be casted from
// BaseApiObjectClass to <AccountType>ApiObjectClass
enum class AccountType {
  LOCAL = 0,
  UNKNOWN = 1,
  NOT_INHERITED = 2,
  INVALID = 3,
  GCAL = 4
};

inline const std::string account_type_to_string(AccountType type) {
  switch (type) {
  case AccountType::LOCAL:
    return std::string("LOCAL");
  case AccountType::UNKNOWN:
    return std::string("Unknown");
  case AccountType::NOT_INHERITED:
    return std::string("NOT_INHERITED");
  case AccountType::INVALID:
    return std::string("INVALID");
  case AccountType::GCAL:
    return std::string("Google Calendar");
  default:
    return std::string(
        "Error: Invalid Account Type, it is not even AccountType::INVALID");
  }
}

} // namespace task_manager

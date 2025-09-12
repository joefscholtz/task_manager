#pragma once
#include "defines.hpp"
#include <string>
namespace task_manager {

class BaseApiEvent {
public:
  BaseApiEvent() {}
  virtual ~BaseApiEvent() = default;

  AccountType get_account_type() const { return this->_account_type; }

private:
  AccountType _account_type = AccountType::UNKNOWN;
};

} // namespace task_manager

#pragma once
#include "defines.hpp"
#include <string>
namespace task_manager {

class BaseApiEvent {
public:
  BaseApiEvent(AccountType account_type = AccountType::UNKNOWN)
      : _account_type(std::move(account_type)) {}

  virtual ~BaseApiEvent() = default;

  // Force all derived classes to implement clone method it.
  // virtual std::unique_ptr<BaseApiEvent> clone() const = 0;

  inline const AccountType &get_account_type() const {
    return this->_account_type;
  }
  inline void set_account_type(const AccountType account_type) {
    this->_account_type = account_type;
  }

private:
  AccountType _account_type;
};

} // namespace task_manager

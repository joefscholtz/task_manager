#pragma once
#include "defines.hpp"
#include "time.hpp"
#include <string>

namespace task_manager {

class Account {
public:
  Account(std::string email, std::string refresh_token = "", uint32_t id = 0)
      : _email(email), _refresh_token(refresh_token), _id(id) {}

  ~Account() = default;

  inline const uint32_t &get_id() const { return this->_id; }
  inline void set_id(uint32_t id) { this->_id = id; }

  inline const AccountType &get_account_type() const {
    return this->_account_type;
  }
  inline void set_account_type(AccountType account_type) {
    this->_account_type = account_type;
  }

  inline const std::string &get_email() const { return this->_email; }
  inline void set_email(std::string email) { this->_email = email; }

  inline const std::string &get_refresh_token() const {
    return this->_refresh_token;
  }
  inline void set_refresh_token(std::string refresh_token) {
    this->_refresh_token = refresh_token;
  }

  friend std::ostream &operator<<(std::ostream &os, const Account &account);

  uint32_t _id;
  AccountType _account_type = AccountType::UNKNOWN;
  std::string _email;
  std::string _refresh_token;
};
} // namespace task_manager

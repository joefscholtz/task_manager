#pragma once
#include "defines.hpp"

#include "BaseApiUserInfo.hpp"
#include "time.hpp"

namespace task_manager {

class Account {
public:
  Account(std::string email = "", std::string refresh_token = "",
          uint32_t id = 0)
      : _email(email), _refresh_token(refresh_token), _id(id) {}

  ~Account() = default;

  inline const uint32_t &get_id() const { return this->_id; }
  inline void set_id(uint32_t id) { this->_id = id; }

  inline const AccountType &get_account_type() const {
    return this->_account_type;
  }
  inline const int &get_account_type_db() const {
    return this->_account_type_db;
  }
  inline void set_account_type(AccountType account_type) {
    this->_account_type = account_type;
    this->_account_type_db = static_cast<int>(account_type);
  }

  inline const std::string &get_email() const { return this->_email; }
  inline void set_email(std::string email) { this->_email = email; }

  inline const BaseApiUserInfo &get_user_info() const {
    return this->_user_info;
  }
  inline void set_user_info(BaseApiUserInfo user_info) {
    this->_user_info = user_info;
  }

  inline const std::string &get_refresh_token() const {
    return this->_refresh_token;
  }
  inline void set_refresh_token(std::string refresh_token) {
    this->_refresh_token = refresh_token;
  }

  // This new method will synchronize your AccountType member
  // after the ORM populates the _start_db and _end_db members.
  void update_members_from_db() {
    this->_account_type = static_cast<AccountType>(this->_account_type_db);
  }

  friend std::ostream &operator<<(std::ostream &os, const Account &account);

  uint32_t _id;
  AccountType _account_type = AccountType::UNKNOWN;
  int _account_type_db = static_cast<int>(AccountType::UNKNOWN);
  BaseApiUserInfo _user_info;
  std::string _email;
  std::string _refresh_token;
};
} // namespace task_manager

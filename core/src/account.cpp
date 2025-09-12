#include "account.hpp"

namespace task_manager {

std::ostream &operator<<(std::ostream &os, const Account &account) {
  os << "Id: " << account.get_id() << "\n"
     << "Account type: " << account_type_to_string(account.get_account_type())
     << "\n"
     << "Email: " << account.get_email() << "\n";
  return os;
}
} // namespace task_manager

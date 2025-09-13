#pragma once

#include "GCalApiEvent.hpp"
#include "GCalApiEventsList.hpp"
#include "GCalApiUserInfo.hpp"
#include "defines.hpp"
#include "time.hpp"
#include <string>

namespace task_manager {
class GoogleCalendarAPI {
public:
  GoogleCalendarAPI(const std::string &secret_path);

  bool authenticate();

  std::optional<std::vector<GCalApiEvent>>
  list_events(int max_results = 100,
              const std::string &refresh_token = std::string());

  time_point parse_gcal_event_datetime(const EventDateTime &event_dt);
  std::optional<std::string>
  get_user_email(const std::string &refresh_token = std::string(""));

  inline GCalApiUserInfo const &get_user_info() { return this->_user_info; }

  bool clear_account();
  inline std::string &get_refresh_token() { return this->_refresh_token; }

private:
  bool load_secrets();
  bool load_tokens_from_file();
  void save_tokens_to_file();
  bool refresh_access_token(const std::string &refresh_token = std::string(),
                            const bool &override_refresh_token = true);
  bool get_tokens_from_auth_code(const std::string &auth_code);

  std::optional<json> make_authenticated_get_request(
      const std::string &url,
      std::optional<cpr::Parameters> params = std::nullopt,
      std::string refresh_token = std::string(""));

  std::string _client_id;
  std::string _client_secret;
  std::string _access_token;
  std::string _refresh_token;

  GCalApiUserInfo _user_info;

  std::string _secret_file_path;
  const std::string _token_file_path = ".env/gcal_token.json";
};

} // namespace task_manager

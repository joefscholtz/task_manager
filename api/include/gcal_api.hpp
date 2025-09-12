#pragma once

#include "GCalApiEvent.hpp"
#include "GCalApiEventsList.hpp"
#include "nlohmann/json.hpp"
#include "time.hpp"
#include <optional>
#include <string>
#include <vector>

namespace task_manager {
class GoogleCalendarAPI {
public:
  GoogleCalendarAPI(const std::string &secret_path);

  bool authenticate();

  std::optional<std::vector<GCalApiEvent>>
  list_events(int max_results = 100,
              const std::string &refresh_token = std::string());

  time_point parse_gcal_event_datetime(const EventDateTime &event_dt);
  std::optional<std::string> get_user_email();

private:
  bool load_secrets();
  bool load_tokens_from_file();
  void save_tokens_to_file();
  bool refresh_access_token(const std::string &refresh_token = std::string(),
                            const bool &override_refresh_token = true);
  bool get_tokens_from_auth_code(const std::string &auth_code);
  bool clear_account();

  std::optional<nlohmann::json> make_authenticated_get_request(
      const std::string &url,
      const std::vector<std::pair<std::string, std::string>> &params,
      const std::string &refresh_token = std::string());

  std::string _client_id;
  std::string _client_secret;
  std::string _access_token;
  std::string _refresh_token;

  std::string _secret_file_path;
  const std::string _token_file_path = ".env/gcal_token.json";
};

} // namespace task_manager

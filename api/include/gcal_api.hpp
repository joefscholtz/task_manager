#pragma once

#include "nlohmann/json.hpp"
#include <optional>
#include <string>
#include <vector>

struct ApiEvent {
  std::string iCalUID;
  std::string summary;
  std::string start_time;
  std::string end_time;
};

class GoogleCalendarAPI {
public:
  GoogleCalendarAPI(const std::string &secret_path);

  bool authenticate();

  std::optional<std::vector<ApiEvent>> list_events(int max_results = 10);

private:
  bool load_secrets();
  bool load_tokens_from_file();
  void save_tokens_to_file();
  bool refresh_access_token();
  bool get_tokens_from_auth_code(const std::string &auth_code);

  std::optional<nlohmann::json> make_authenticated_get_request(
      const std::string &url,
      const std::vector<std::pair<std::string, std::string>> &params);

  std::string _client_id;
  std::string _client_secret;
  std::string _access_token;
  std::string _refresh_token;

  std::string _secret_file_path;
  const std::string _token_file_path = ".env/gcal_token.json";
};

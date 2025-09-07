#pragma once

#include "nlohmann/json.hpp"
#include <optional>
#include <string>
#include <vector>

// A simple struct to hold event data returned from the API
// This avoids a dependency on your core Event class.
struct ApiEvent {
  std::string id;
  std::string summary;
  std::string start_time;
  std::string end_time;
};

class GoogleCalendarAPI {
public:
  GoogleCalendarAPI(const std::string &secret_path);

  // Initiates the authentication flow. Returns false if secrets can't be
  // loaded.
  bool authenticate();

  // Fetches a list of upcoming events.
  std::optional<std::vector<ApiEvent>> list_events(int max_results = 10);

private:
  // Helper methods for the OAuth 2.0 flow
  bool load_secrets();
  bool load_tokens_from_file();
  void save_tokens_to_file();
  bool refresh_access_token();
  bool get_tokens_from_auth_code(const std::string &auth_code);

  // A general-purpose helper to make authenticated GET requests
  std::optional<nlohmann::json> make_authenticated_get_request(
      const std::string &url,
      const std::vector<std::pair<std::string, std::string>> &params);

  // Member variables
  std::string _client_id;
  std::string _client_secret;
  std::string _access_token;
  std::string _refresh_token;

  std::string _secret_file_path;
  const std::string _token_file_path = "gcal_token.json";
};

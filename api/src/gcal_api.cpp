#include "gcal_api.hpp"
#include <chrono>
#include <cpr/cpr.h>
#include <format>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

GoogleCalendarAPI::GoogleCalendarAPI(const std::string &secret_path)
    : _secret_file_path(secret_path) {
  load_secrets();
  load_tokens_from_file();
}

bool GoogleCalendarAPI::load_secrets() {
  std::ifstream f(_secret_file_path);
  if (!f.is_open()) {
    std::cerr << "Error: Could not open client_secret.json" << std::endl;
    return false;
  }
  json secrets = json::parse(f);
  _client_id = secrets["installed"]["client_id"];
  _client_secret = secrets["installed"]["client_secret"];
  return true;
}

bool GoogleCalendarAPI::load_tokens_from_file() {
  std::ifstream f(_token_file_path);
  if (!f.is_open()) {
    return false; // No token file yet, this is normal on first run
  }
  json tokens = json::parse(f);
  _access_token = tokens.value("access_token", "");
  _refresh_token = tokens.value("refresh_token", "");
  return !_refresh_token.empty();
}

void GoogleCalendarAPI::save_tokens_to_file() {
  json tokens = {{"access_token", _access_token},
                 {"refresh_token", _refresh_token}};
  std::ofstream o(_token_file_path);
  o << std::setw(4) << tokens << std::endl;
}

bool GoogleCalendarAPI::get_tokens_from_auth_code(
    const std::string &auth_code) {
  cpr::Response r =
      cpr::Post(cpr::Url{"https://oauth2.googleapis.com/token"},
                cpr::Payload{{"client_id", _client_id},
                             {"client_secret", _client_secret},
                             {"code", auth_code},
                             {"grant_type", "authorization_code"},
                             {"redirect_uri", "urn:ietf:wg:oauth:2.0:oob"}});
  if (r.status_code != 200) {
    std::cerr << "Error getting access token: " << r.text << std::endl;
    return false;
  }
  json result = json::parse(r.text);
  _access_token = result["access_token"];
  _refresh_token = result["refresh_token"];
  save_tokens_to_file();
  return true;
}

bool GoogleCalendarAPI::authenticate() {
  if (_client_id.empty())
    return false;

  std::string scope = "https://www.googleapis.com/auth/calendar.events";
  std::string auth_url = "https://accounts.google.com/o/oauth2/v2/auth?"
                         "scope=" +
                         scope +
                         "&"
                         "response_type=code&"
                         "redirect_uri=urn:ietf:wg:oauth:2.0:oob&"
                         "client_id=" +
                         _client_id;

  std::cout << "Please go to this URL to authorize the application:\n"
            << auth_url << std::endl;
  std::cout << "\nEnter the authorization code you received: ";
  std::string auth_code;
  std::cin >> auth_code;

  if (get_tokens_from_auth_code(auth_code)) {
    std::cout << "\nAuthentication successful! Tokens saved to "
              << _token_file_path << std::endl;
    return true;
  }
  return false;
}

bool GoogleCalendarAPI::refresh_access_token() {
  if (_refresh_token.empty()) {
    std::cerr << "No refresh token available. Please authenticate again."
              << std::endl;
    return false;
  }
  cpr::Response r = cpr::Post(cpr::Url{"https://oauth2.googleapis.com/token"},
                              cpr::Payload{{"client_id", _client_id},
                                           {"client_secret", _client_secret},
                                           {"refresh_token", _refresh_token},
                                           {"grant_type", "refresh_token"}});
  if (r.status_code != 200) {
    std::cerr << "Error refreshing access token: " << r.text << std::endl;
    return false;
  }
  json result = json::parse(r.text);
  _access_token = result["access_token"];
  // Note: A new refresh token is sometimes returned, but often not.
  // It's good practice to save the new one if it exists.
  if (result.contains("refresh_token")) {
    _refresh_token = result["refresh_token"];
  }
  save_tokens_to_file();
  std::cout << "Access token refreshed." << std::endl;
  return true;
}

std::optional<json> GoogleCalendarAPI::make_authenticated_get_request(
    const std::string &url,
    const std::vector<std::pair<std::string, std::string>> &params) {
  auto send_request = [&]() {
    cpr::Parameters cpr_params;
    for (const auto &p : params) {
      cpr_params.Add({p.first, p.second});
    }
    return cpr::Get(cpr::Url{url}, cpr_params,
                    cpr::Header{{"Authorization", "Bearer " + _access_token}});
  };

  auto r = send_request();
  if (r.status_code == 401) { // 401 Unauthorized, token likely expired
    std::cout << "Access token expired. Attempting to refresh..." << std::endl;
    if (refresh_access_token()) {
      r = send_request(); // Retry the request with the new token
    }
  }

  if (r.status_code != 200) {
    std::cerr << "API request failed with status " << r.status_code << ": "
              << r.text << std::endl;
    return std::nullopt;
  }

  return json::parse(r.text);
}

std::optional<std::vector<ApiEvent>>
GoogleCalendarAPI::list_events(int max_results) {
  if (_access_token.empty() && !refresh_access_token()) {
    std::cout << "Authentication required. Please run the 'gcal_login' command."
              << std::endl;
    return std::nullopt;
  }

  auto now = std::chrono::system_clock::now();
  auto time_str = std::format("{:%Y-%m-%dT%H:%M:%SZ}", now);

  auto result_json = make_authenticated_get_request(
      "https://www.googleapis.com/calendar/v3/calendars/primary/events",
      {{"maxResults", std::to_string(max_results)},
       {"orderBy", "startTime"},
       {"singleEvents", "true"},
       {"timeMin", time_str}});

  if (!result_json) {
    return std::nullopt;
  }

  std::vector<ApiEvent> events;
  for (const auto &item : (*result_json)["items"]) {
    ApiEvent ev;
    ev.id = item.value("id", "");
    ev.summary = item.value("summary", "No Title");
    ev.start_time =
        item["start"].value("dateTime", item["start"].value("date", ""));
    ev.end_time = item["end"].value("dateTime", item["end"].value("date", ""));
    events.push_back(ev);
  }

  return events;
}

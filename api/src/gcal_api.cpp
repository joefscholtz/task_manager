#include "gcal_api.hpp"
#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

namespace task_manager {

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
    std::cout << "\nAuthentication successful!" << std::endl;
    return true;
  }
  return false;
}

bool GoogleCalendarAPI::refresh_access_token(
    const std::string &refresh_token, const bool &override_refresh_token) {
  if (!refresh_token.empty() && override_refresh_token)
    _refresh_token = refresh_token;

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
  if (result.contains("refresh_token")) {
    _refresh_token = result["refresh_token"];
  }
  std::cout << "Access token refreshed." << std::endl;
  return true;
}

std::optional<json> GoogleCalendarAPI::make_authenticated_get_request(
    const std::string &url, std::optional<cpr::Parameters> params,
    std::string refresh_token) {

  auto send_request = [&]() {
    cpr::Parameters cpr_params = params.value_or(cpr::Parameters{});

    return cpr::Get(cpr::Url{url}, cpr_params,
                    cpr::Header{{"Authorization", "Bearer " + _access_token}});
  };

  auto r = send_request();

  if (r.status_code == 401) {
    std::cout << "Access token expired. Attempting to refresh..." << std::endl;
    if (refresh_access_token(refresh_token)) {
      r = send_request();
    }
  }

  if (r.status_code != 200) {
    std::cerr << "API request failed with status " << r.status_code << ": "
              << r.text << std::endl;
    return std::nullopt; // Return an empty optional on failure.
  }

  try {
    return json::parse(r.text);
  } catch (const json::exception &e) {
    std::cerr << "Failed to parse JSON response: " << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<std::vector<GCalApiEvent>>
GoogleCalendarAPI::list_events(int max_results,
                               const std::string &refresh_token) {
  if (_access_token.empty() && !refresh_access_token(refresh_token)) {
    std::cout << "Authentication required. Please run the 'gcal_login' command."
              << std::endl;
    return std::nullopt;
  }

  auto now = std::chrono::system_clock::now();
  auto time_str = std::format("{:%Y-%m-%dT%H:%M:%SZ}", now);

  auto result_json = make_authenticated_get_request(
      "https://www.googleapis.com/calendar/v3/calendars/primary/events",
      cpr::Parameters{{"maxResults", std::to_string(max_results)},
                      {"orderBy", "startTime"},
                      {"singleEvents", "true"},
                      {"timeMin", time_str}},
      this->_refresh_token);

  if (!result_json) {
    return std::nullopt;
  }

  std::vector<GCalApiEvent> events;
  GCalApiEventsList events_list = result_json->get<GCalApiEventsList>();
  events = events_list.items;

  return events;
}

time_point
GoogleCalendarAPI::parse_gcal_event_datetime(const EventDateTime &event_dt) {
  // Case 1: It's a timed event with a specific dateTime
  if (!event_dt.dateTime.empty()) {
    std::istringstream in{event_dt.dateTime};
    time_point tp;
    // The "%Z" or "%z" specifier correctly parses the timezone offset
    in >> std::chrono::parse("%Y-%m-%dT%H:%M:%S%z", tp);
    if (!in.fail()) {
      return tp;
    }
  }

  // Case 2: It's an all-day event with only a date
  if (!event_dt.date.empty()) {
    std::istringstream in{event_dt.date +
                          "T00:00:00Z"}; // Treat as start of day in UTC
    time_point tp;
    in >> std::chrono::parse("%Y-%m-%dT%H:%M:%S%z", tp);
    if (!in.fail()) {
      return tp;
    }
  }

  // Case 3: Both are empty or parsing failed
  return {}; // Return an empty/default time_point
}

bool GoogleCalendarAPI::clear_account() {
  _access_token = std::string();
  _refresh_token = std::string();
  _user_info = GCalApiUserInfo();
  return true;
}

std::optional<std::string>
GoogleCalendarAPI::get_user_email(const std::string &refresh_token) {
  if (!refresh_access_token(refresh_token)) {
    std::cout << "Authentication required. Please login again." << std::endl;
    return std::nullopt;
  }
  auto result_json = make_authenticated_get_request(
      "https://www.googleapis.com/oauth2/v3/userinfo", std::nullopt,
      this->_refresh_token);

  if (!result_json) {
    return std::nullopt;
  }

  try {
    GCalApiUserInfo user_info = result_json->get<GCalApiUserInfo>();
    _user_info = user_info;
    return user_info.email;
  } catch (const json::exception &e) {
    std::cerr << "Error parsing user info response: " << e.what() << std::endl;
    return std::nullopt; // Return empty if email wasn't found or parsing failed
  }
}

} // namespace task_manager

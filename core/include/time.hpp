#pragma once
#include "defines.hpp"
#include <chrono>
#include <format>
#include <iomanip> // for std::get_time

using namespace std::literals;

time_point parse_datetime(const std::string &datetime_str) {
  if (datetime_str.empty())
    return {};

  std::tm tm = {};
  std::istringstream ss(datetime_str);

  // Google's format is like "2025-09-08T10:00:00-03:00"
  // TODO: make this generic and add parse_datetime_google function
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

  return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

#pragma once
#include "defines.hpp"
#include <chrono>
#include <iomanip> // for std::get_time

namespace task_manager {

using namespace std::literals;
using time_point = std::chrono::system_clock::time_point;

} // namespace task_manager

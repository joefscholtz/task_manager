cmake_minimum_required(VERSION 3.15)
project(task_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)


if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Enable folders in IDEs (VSCode, CLion, etc.)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(3rd_party)
add_subdirectory(core)
# add_subdirectory(api)

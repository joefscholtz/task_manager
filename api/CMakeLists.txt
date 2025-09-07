find_package(cpr REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(api src/gcal_api.cpp)
target_include_directories(api PUBLIC include)
target_link_libraries(api PRIVATE cpr::cpr nlohmann_json::nlohmann_json)

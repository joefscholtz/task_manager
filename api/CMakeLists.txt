find_package(cpr REQUIRED)

add_library(api src/gcal_api.cpp)
target_include_directories(api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(api PRIVATE third_party cpr::cpr)

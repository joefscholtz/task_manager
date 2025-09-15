find_package(sdbus-cpp REQUIRED)

add_executable(taks_manager_daemon
    src/main.cpp
)
target_include_directories(taks_manager_daemon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(taks_manager_daemon PRIVATE shared_includes third_party sdbus-cpp)

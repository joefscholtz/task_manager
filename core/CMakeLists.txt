add_library(task_manager_core STATIC
    src/account.cpp
    src/calendar.cpp
    src/event.cpp
)

target_include_directories(task_manager_core 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(task_manager_core PUBLIC shared_includes third_party api)

add_executable(task_manager_cli
    src/cli.cpp
)

target_link_libraries(task_manager_cli PRIVATE task_manager_core)

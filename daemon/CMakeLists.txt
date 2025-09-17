find_package(pugixml CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED systemd)

add_executable(task_manager_daemon
    src/main.cpp
    src/daemon.cpp
)
target_include_directories(task_manager_daemon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(task_manager_daemon PRIVATE 
                      task_manager_core
                      pugixml::static
                      pugixml::pugixml
                      PkgConfig::Systemd
)

find_package(pugixml CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED systemd)

set(DBUS_XML ${CMAKE_CURRENT_SOURCE_DIR}/config/org.task_manager.Daemon.xml)
set(GENERATED_ADAPTOR ${CMAKE_CURRENT_BINARY_DIR}/daemon_adaptor.h)

add_custom_command(
    OUTPUT ${GENERATED_ADAPTOR}
    COMMAND SDBusCpp::sdbus-c++-xml2cpp ${DBUS_XML} --adaptor=${GENERATED_ADAPTOR}
    DEPENDS ${DBUS_XML}
    COMMENT "Generating D-Bus adaptor for task_manager_daemon"
)

add_executable(task_manager_daemon
    src/main.cpp
    src/daemon.cpp
    ${GENERATED_ADAPTOR}
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

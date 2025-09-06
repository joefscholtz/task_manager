# find_package(SQLite3 REQUIRED)
# add_library(core)
#
# target_sources(task_manager_cli
#     PRIVATE
#         src/**.cpp
#     PUBLIC
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
# )
#
# target_include_directories(task_manager_cli
#     PUBLIC
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#         $<INSTALL_INTERFACE:include>
# )
#
# #target_link_libraries(core PUBLIC )
#
# set_target_properties(task_manager_cli PROPERTIES
#     OUTPUT_NAME "task_manager_cli"
# )

add_executable(task_manager_cli)

target_sources(task_manager_cli
    PRIVATE
        src/calendar.cpp
        src/cli.cpp
)

target_include_directories(task_manager_cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(task_manager_cli
    PRIVATE
        replxx
)

set_target_properties(task_manager_cli PROPERTIES
    OUTPUT_NAME "task_manager_cli"
)

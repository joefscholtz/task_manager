# Create the main INTERFACE library at the very beginning.
add_library(third_party INTERFACE)

# Provided by vcpkg
find_package(cpr REQUIRED)
find_package(nlohmann_json 3.11.0 REQUIRED)
find_package(replxx)
target_link_libraries(third_party INTERFACE 
    cpr::cpr
    nlohmann_json::nlohmann_json
    replxx::replxx
)

# ===============================
# sqlite_orm 
# ===============================
find_package(SQLite3 REQUIRED)
add_library(sqlite_orm INTERFACE)
target_include_directories(sqlite_orm INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite_orm/include
)
set(SQLITE_ORM_OPTIONAL_SUPPORTED ON)
target_link_libraries(third_party INTERFACE sqlite_orm SQLite::SQLite3)

# ===============================
# cpp_json_class_gen 
# ===============================
# This project is a tool and doesn't need to be linked into the main library.
# It just needs to be built so other parts of the project can use it.
add_subdirectory(cpp_json_class_gen)


# ===============================
# cpp_json_class_gen 
# ===============================
add_subdirectory(sdbus-cpp)
target_link_libraries(third_party INTERFACE sdbus-c++)


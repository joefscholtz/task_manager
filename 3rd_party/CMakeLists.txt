# ===============================
# Replxx
# ===============================
add_subdirectory(replxx)

# ===============================
# sqlite_orm 
# ===============================
find_package(SQLite3 REQUIRED)

add_library(sqlite_orm INTERFACE)
target_include_directories(sqlite_orm INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite_orm/include
)
# ===============================
# json 
# ===============================
add_subdirectory(json)

# ===============================
# export library
# ===============================
add_library(third_party INTERFACE)
target_link_libraries(third_party
    INTERFACE
        nlohmann_json
        replxx
        sqlite_orm
        SQLite::SQLite3
)

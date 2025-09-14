find_package(cpr REQUIRED)
find_package(nlohmann_json 3.11.0 REQUIRED)
# find_package(cpp_json_class_gen REQUIRED)

add_library(api STATIC
    src/gcal_api.cpp
)
target_include_directories(api 
    # PUBLIC: Consumers of 'api' need access to the public headers.
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    # PUBLIC: Consumers also need access to the generated headers in the build directory.
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

generate_class_from_json(api
  CONFIGS
  meta_schemas/GCalApiEvent.json
  meta_schemas/GCalApiEventsList.json
  meta_schemas/GCalApiUserInfo.json
)

target_link_libraries(api PRIVATE shared_includes third_party cpr::cpr)

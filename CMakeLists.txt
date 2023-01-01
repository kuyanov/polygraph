cmake_minimum_required(VERSION 3.16)
project(polygraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_TSAN "-g -fsanitize=thread")
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY TRUE)

add_compile_definitions(
        SCHEMA_DIR="${CMAKE_SOURCE_DIR}/schema"
        ROOT_DIR="${CMAKE_SOURCE_DIR}/shared"
)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_SUBMODULES ""
        GIT_PROGRESS TRUE
)
FetchContent_Declare(
        uWebSockets
        GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
        GIT_TAG v20.22.0
        GIT_SUBMODULES uSockets
        GIT_SUBMODULES_RECURSE FALSE
        GIT_PROGRESS TRUE
)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
        GIT_PROGRESS TRUE
)

FetchContent_GetProperties(rapidjson)
if (NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif ()

include_directories("${rapidjson_SOURCE_DIR}/include")

add_subdirectory(common)
add_subdirectory(structures)

if (ENABLE_CLIENT)
    add_subdirectory(client)
endif ()
if (ENABLE_RUNNER)
    add_subdirectory(runner)
endif ()
if (ENABLE_SCHEDULER)
    add_subdirectory(scheduler)
endif ()
if (ENABLE_TESTING)
    add_subdirectory(test)
endif ()

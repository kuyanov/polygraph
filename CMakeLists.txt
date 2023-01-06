cmake_minimum_required(VERSION 3.16)
project(polygraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_TSAN "-g -fsanitize=thread")
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY TRUE)

include(FetchContent)
include(GNUInstallDirs)

find_package(Boost REQUIRED COMPONENTS program_options)

add_compile_definitions(
        DEFAULT_DATA_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}"
        DEFAULT_VAR_DIR="${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/${PROJECT_NAME}"
)

set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_SUBMODULES ""
)
FetchContent_Declare(
        uWebSockets
        GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
        GIT_TAG v20.22.0
        GIT_SUBMODULES uSockets
        GIT_SUBMODULES_RECURSE FALSE
)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)

FetchContent_GetProperties(rapidjson)
if (NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif ()

include_directories(${rapidjson_SOURCE_DIR}/include)

add_subdirectory(common)

if (NOT DISABLE_CLIENT)
    add_subdirectory(client)
endif ()
if (NOT DISABLE_RUNNER)
    add_subdirectory(runner)
endif ()
if (NOT DISABLE_SCHEDULER)
    add_subdirectory(scheduler)
endif ()
if (NOT DISABLE_TESTING)
    add_subdirectory(test)
endif ()

install(DIRECTORY data/ DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/${PROJECT_NAME}/user)

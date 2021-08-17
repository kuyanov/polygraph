cmake_minimum_required(VERSION 3.17)
project(PolyGraph)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-fsanitize=address")
    add_link_options("-fsanitize=address")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options("-Ofast")
endif()

add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE rapidjson/include)

if (MASTERNODE)
    add_subdirectory(masternode)
endif()

if (WORKER)
    add_subdirectory(worker)
endif()

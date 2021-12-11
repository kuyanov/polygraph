cmake_minimum_required(VERSION 3.17)
project(PolyGraph)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all")

add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE contrib/rapidjson/include)

if (MASTERNODE)
    add_subdirectory(masternode)
endif()

if (RUNNER)
    add_subdirectory(runner)
endif()

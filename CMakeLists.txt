cmake_minimum_required(VERSION 3.16)
project(polygraph)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined -fno-sanitize-recover=all")
set(CMAKE_CXX_FLAGS_TSAN "-g -fsanitize=thread")

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE contrib/rapidjson/include)

add_subdirectory(masternode)
add_subdirectory(runner)

add_subdirectory(contrib/googletest)
add_subdirectory(test)

cmake_minimum_required(VERSION 3.16)
project(polygraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_TSAN "-g -fsanitize=thread")

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)
find_library(USOCKETS_LIB uSockets.a REQUIRED)

add_library(uWebSockets INTERFACE)
target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
target_link_libraries(uWebSockets INTERFACE ${USOCKETS_LIB})

add_subdirectory(common)
add_subdirectory(runner)
add_subdirectory(scheduler)
add_subdirectory(test)

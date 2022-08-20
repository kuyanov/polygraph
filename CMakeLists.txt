cmake_minimum_required(VERSION 3.16)
project(polygraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_TSAN "-g -fsanitize=thread")

find_library(USOCKETS_LIBRARY uSockets.a REQUIRED)

add_library(uWebSockets INTERFACE)
target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
target_link_libraries(uWebSockets INTERFACE ${USOCKETS_LIBRARY})

add_subdirectory(common)
add_subdirectory(runner)
add_subdirectory(scheduler)
add_subdirectory(test)

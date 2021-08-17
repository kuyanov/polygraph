cmake_minimum_required(VERSION 3.17)
project(PolyGraph)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options("-fsanitize=address")
    add_link_options("-fsanitize=address")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options("-Ofast")
endif()

add_library(uSockets)
file(GLOB USOCKETS_SOURCES
        uWebSockets/uSockets/src/*.c
        uWebSockets/uSockets/src/eventing/*.c
        uWebSockets/uSockets/src/crypto/*.c*)
target_sources(uSockets PRIVATE ${USOCKETS_SOURCES})
target_include_directories(uSockets PUBLIC uWebSockets/uSockets/src)

add_library(uWebSockets INTERFACE)
target_include_directories(uWebSockets INTERFACE uWebSockets/src)

if (WITH_OPENSSL)
    find_package(OpenSSL REQUIRED)
    target_include_directories(uSockets PUBLIC ${OPENSSL_INCLUDE_DIR})
    target_compile_definitions(uSockets PRIVATE LIBUS_USE_OPENSSL)
    target_link_libraries(uSockets PRIVATE OpenSSL::SSL OpenSSL::Crypto)
else()
    target_compile_definitions(uSockets PRIVATE LIBUS_NO_SSL)
endif()

if (WITH_LIBUV)
    find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
    find_library(LIBUV_LIBRARY NAMES uv libuv)
    target_include_directories(uSockets PUBLIC ${LIBUV_INCLUDE_DIR})
    target_compile_definitions(uSockets PRIVATE LIBUS_USE_LIBUV)
    target_link_libraries(uSockets PRIVATE ${LIBUV_LIBRARY})
endif()

if (WITH_ZLIB)
    find_package(ZLIB REQUIRED)
    target_include_directories(uSockets PUBLIC ${ZLIB_INCLUDE_DIR})
    target_link_libraries(uSockets PRIVATE ${ZLIB_LIBRARY})
else()
    target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
endif()

target_link_libraries(uWebSockets INTERFACE uSockets)

add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE rapidjson/include)

add_executable(MasterNode masternode/main.cpp)
target_link_libraries(MasterNode PRIVATE uWebSockets RapidJSON)

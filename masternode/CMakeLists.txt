set(USOCKETS_SRC_DIR ../contrib/uWebSockets/uSockets/src)
set(UWEBSOCKETS_SRC_DIR ../contrib/uWebSockets/src)

add_library(uSockets)
file(GLOB USOCKETS_SOURCES
        ${USOCKETS_SRC_DIR}/*.c
        ${USOCKETS_SRC_DIR}/eventing/*.c
        ${USOCKETS_SRC_DIR}/crypto/*.c*)
target_sources(uSockets PRIVATE ${USOCKETS_SOURCES})
target_include_directories(uSockets PUBLIC ${USOCKETS_SRC_DIR})

add_library(uWebSockets INTERFACE)
target_include_directories(uWebSockets INTERFACE ${UWEBSOCKETS_SRC_DIR})

if (USE_OPENSSL)
    find_package(OpenSSL REQUIRED)
    target_include_directories(uSockets PUBLIC ${OPENSSL_INCLUDE_DIR})
    target_compile_definitions(uSockets PRIVATE LIBUS_USE_OPENSSL)
    target_link_libraries(uSockets PRIVATE OpenSSL::SSL OpenSSL::Crypto)
else()
    target_compile_definitions(uSockets PRIVATE LIBUS_NO_SSL)
endif()

if (USE_LIBUV)
    find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
    find_library(LIBUV_LIBRARY NAMES uv libuv)
    target_include_directories(uSockets PUBLIC ${LIBUV_INCLUDE_DIR})
    target_compile_definitions(uSockets PRIVATE LIBUS_USE_LIBUV)
    target_link_libraries(uSockets PRIVATE ${LIBUV_LIBRARY})
endif()

if (USE_ZLIB)
    find_package(ZLIB REQUIRED)
    target_include_directories(uSockets PUBLIC ${ZLIB_INCLUDE_DIR})
    target_link_libraries(uSockets PRIVATE ${ZLIB_LIBRARY})
else()
    target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
endif()

target_link_libraries(uWebSockets INTERFACE uSockets)

add_library(MasterNodeLib)
file(GLOB MASTERNODE_SOURCES src/*.cpp)
target_sources(MasterNodeLib PRIVATE ${MASTERNODE_SOURCES})
target_include_directories(MasterNodeLib PUBLIC src)
target_link_libraries(MasterNodeLib PRIVATE uWebSockets RapidJSON)

add_executable(MasterNode main.cpp)
target_link_libraries(MasterNode PRIVATE MasterNodeLib)

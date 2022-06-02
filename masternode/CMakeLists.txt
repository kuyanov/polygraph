set(USOCKETS_SRC_DIR ${CMAKE_SOURCE_DIR}/contrib/uWebSockets/uSockets/src)
set(UWEBSOCKETS_SRC_DIR ${CMAKE_SOURCE_DIR}/contrib/uWebSockets/src)

find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARY NAMES uv libuv)

add_library(uSockets)
file(GLOB USOCKETS_SOURCES
        ${USOCKETS_SRC_DIR}/*.c
        ${USOCKETS_SRC_DIR}/eventing/*.c
        ${USOCKETS_SRC_DIR}/crypto/*.c*)
target_sources(uSockets PRIVATE ${USOCKETS_SOURCES})
target_compile_definitions(uSockets PRIVATE LIBUS_NO_SSL LIBUS_USE_LIBUV)
target_include_directories(uSockets PUBLIC ${LIBUV_INCLUDE_DIR} ${USOCKETS_SRC_DIR})
target_link_libraries(uSockets PRIVATE Threads::Threads ${LIBUV_LIBRARY})

if (WIN32)
    target_link_libraries(uSockets PRIVATE userenv iphlpapi)
endif ()

add_library(uWebSockets INTERFACE)
target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
target_include_directories(uWebSockets INTERFACE ${UWEBSOCKETS_SRC_DIR})
target_link_libraries(uWebSockets INTERFACE uSockets)

add_library(MasterNodeLib)
file(GLOB MASTERNODE_SOURCES src/*.cpp)
target_sources(MasterNodeLib PRIVATE ${MASTERNODE_SOURCES})
target_compile_definitions(MasterNodeLib PUBLIC
        SCHEMA_DIR="${CMAKE_SOURCE_DIR}/masternode/schema"
        SANDBOX_DIR="${CMAKE_SOURCE_DIR}/sandbox"
        USER_DIR="${CMAKE_SOURCE_DIR}/user")
target_include_directories(MasterNodeLib PUBLIC src)
target_link_libraries(MasterNodeLib PRIVATE uWebSockets RapidJSON)

add_executable(MasterNode main.cpp)
add_compile_definitions(CONFIG_FILE="${CMAKE_SOURCE_DIR}/masternode/config.json")
target_link_libraries(MasterNode PRIVATE MasterNodeLib)

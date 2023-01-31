FetchContent_GetProperties(uWebSockets)
if (NOT uwebsockets_POPULATED)
    FetchContent_Populate(uWebSockets)
endif ()

set(usockets_SOURCE_DIR ${uwebsockets_SOURCE_DIR}/uSockets)

add_custom_command(
        COMMAND make
        WORKING_DIRECTORY ${usockets_SOURCE_DIR}
        OUTPUT ${usockets_SOURCE_DIR}/uSockets.a
)
add_custom_target(uSockets_build DEPENDS ${usockets_SOURCE_DIR}/uSockets.a)

add_library(uSockets STATIC IMPORTED)
set_target_properties(
        uSockets PROPERTIES
        IMPORTED_LOCATION ${usockets_SOURCE_DIR}/uSockets.a
        INTERFACE_INCLUDE_DIRECTORIES ${usockets_SOURCE_DIR}/src
)
add_dependencies(uSockets uSockets_build)

add_library(uWebSockets INTERFACE)
target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
target_include_directories(uWebSockets INTERFACE ${uwebsockets_SOURCE_DIR}/src)
target_link_libraries(uWebSockets INTERFACE uSockets)

add_executable(pscheduler main.cpp)
file(GLOB PSCHEDULER_SOURCES src/*.cpp)
target_sources(pscheduler PRIVATE ${PSCHEDULER_SOURCES})
target_include_directories(pscheduler PUBLIC src)
target_link_libraries(pscheduler PRIVATE common uWebSockets)

install(TARGETS pscheduler
        DESTINATION libexec
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ)

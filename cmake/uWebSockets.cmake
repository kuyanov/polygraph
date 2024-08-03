include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
        uWebSockets
        GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
        GIT_TAG v20.64.0
        GIT_SUBMODULES uSockets
        GIT_SUBMODULES_RECURSE FALSE
)
FetchContent_MakeAvailable(uWebSockets)

set(usockets_SOURCE_DIR "${uwebsockets_SOURCE_DIR}/uSockets")

add_custom_command(
        COMMAND "make"
        WORKING_DIRECTORY "${usockets_SOURCE_DIR}"
        OUTPUT "${usockets_SOURCE_DIR}/uSockets.a"
)
add_custom_target(uSocketsBuild DEPENDS "${usockets_SOURCE_DIR}/uSockets.a")

add_library(uSockets STATIC IMPORTED)
set_target_properties(
        uSockets PROPERTIES
        IMPORTED_LOCATION "${usockets_SOURCE_DIR}/uSockets.a"
        INTERFACE_INCLUDE_DIRECTORIES "${usockets_SOURCE_DIR}/src"
)
add_dependencies(uSockets uSocketsBuild)

add_library(uWebSockets INTERFACE)
target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
target_include_directories(uWebSockets INTERFACE "${uwebsockets_SOURCE_DIR}/src")
target_link_libraries(uWebSockets INTERFACE uSockets)

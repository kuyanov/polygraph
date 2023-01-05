FetchContent_GetProperties(uWebSockets)
if (NOT uwebsockets_POPULATED)
    FetchContent_Populate(uWebSockets)
endif ()

set(usockets_SOURCE_DIR "${uwebsockets_SOURCE_DIR}/uSockets")

add_custom_command(
        COMMAND "make"
        WORKING_DIRECTORY "${usockets_SOURCE_DIR}"
        OUTPUT "${usockets_SOURCE_DIR}/uSockets.a"
)
add_custom_target(uSockets_build DEPENDS "${usockets_SOURCE_DIR}/uSockets.a")

add_library(uSockets STATIC IMPORTED)
set_target_properties(
        uSockets PROPERTIES
        IMPORTED_LOCATION "${usockets_SOURCE_DIR}/uSockets.a"
        INTERFACE_INCLUDE_DIRECTORIES "${usockets_SOURCE_DIR}/src"
)
add_dependencies(uSockets uSockets_build)

add_library(uWebSockets INTERFACE)
target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
target_include_directories(uWebSockets INTERFACE "${uwebsockets_SOURCE_DIR}/src")
target_link_libraries(uWebSockets INTERFACE uSockets)

add_library(scheduler_impl)
file(GLOB SCHEDULER_SOURCES src/*.cpp)
target_sources(scheduler_impl PRIVATE ${SCHEDULER_SOURCES})
target_include_directories(scheduler_impl PUBLIC src)
target_link_libraries(scheduler_impl PUBLIC common uWebSockets)

add_executable(scheduler main.cpp)
target_compile_definitions(scheduler PRIVATE
        DEFAULT_CONFIG_PATH="${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}/scheduler_conf.json")
target_link_libraries(scheduler PRIVATE ${Boost_LIBRARIES} scheduler_impl)

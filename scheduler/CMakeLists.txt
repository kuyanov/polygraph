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

add_library(SchedulerLib)
file(GLOB SCHEDULER_SOURCES src/*.cpp)
target_sources(SchedulerLib PRIVATE ${SCHEDULER_SOURCES})
target_compile_definitions(SchedulerLib PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/scheduler/config.json")
target_include_directories(SchedulerLib PUBLIC src)
target_link_libraries(SchedulerLib PUBLIC Common Structures uWebSockets)

add_executable(Scheduler main.cpp)
target_link_libraries(Scheduler PRIVATE SchedulerLib)

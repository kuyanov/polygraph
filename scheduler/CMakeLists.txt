find_library(USOCKETS_LIBRARY uSockets.a REQUIRED)

add_library(uWebSockets INTERFACE)
target_compile_definitions(uWebSockets INTERFACE UWS_NO_ZLIB)
target_link_libraries(uWebSockets INTERFACE ${USOCKETS_LIBRARY})

add_executable(Scheduler main.cpp)
file(GLOB SCHEDULER_SOURCES src/*.cpp)
target_sources(Scheduler PRIVATE ${SCHEDULER_SOURCES})
target_compile_definitions(Scheduler PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/scheduler/config.json")
target_include_directories(Scheduler PUBLIC src)
target_link_libraries(Scheduler PRIVATE Common Structures uWebSockets)

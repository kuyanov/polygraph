add_library(SchedulerLib)
file(GLOB SCHEDULER_SOURCES src/*.cpp)
target_sources(SchedulerLib PRIVATE ${SCHEDULER_SOURCES})
target_include_directories(SchedulerLib PUBLIC src)
target_link_libraries(SchedulerLib PRIVATE Common uWebSockets)

add_executable(Scheduler main.cpp)
add_compile_definitions(CONFIG_FILE="${CMAKE_SOURCE_DIR}/scheduler/config.json")
target_link_libraries(Scheduler PRIVATE SchedulerLib)

add_executable(polygraph-scheduler main.cpp)
file(GLOB SCHEDULER_SOURCES src/*.cpp)
target_sources(polygraph-scheduler PRIVATE ${SCHEDULER_SOURCES})
target_include_directories(polygraph-scheduler PUBLIC src)
target_link_libraries(polygraph-scheduler PRIVATE common uWebSockets)

install(TARGETS polygraph-scheduler
        DESTINATION libexec
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ)

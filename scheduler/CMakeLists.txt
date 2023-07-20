add_executable(pscheduler main.cpp)
file(GLOB PSCHEDULER_SOURCES src/*.cpp)
target_sources(pscheduler PRIVATE ${PSCHEDULER_SOURCES})
target_include_directories(pscheduler PUBLIC src)
target_link_libraries(pscheduler PRIVATE common uWebSockets)

install(TARGETS pscheduler
        DESTINATION libexec
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ)

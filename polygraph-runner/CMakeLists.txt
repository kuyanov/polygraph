find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)

add_executable(polygraph-runner main.cpp)
file(GLOB RUNNER_SOURCES src/*.cpp)
target_sources(polygraph-runner PRIVATE ${RUNNER_SOURCES})
target_include_directories(polygraph-runner PUBLIC src)
target_link_libraries(polygraph-runner PRIVATE common ${LIBSBOX_LIBRARY})

install(TARGETS polygraph-runner
        DESTINATION libexec
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ)

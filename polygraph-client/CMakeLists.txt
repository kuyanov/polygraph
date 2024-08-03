add_executable(polygraph-client main.cpp)
file(GLOB CLIENT_SOURCES src/*.cpp)
target_sources(polygraph-client PRIVATE ${CLIENT_SOURCES})
target_include_directories(polygraph-client PUBLIC src)
target_link_libraries(polygraph-client PRIVATE common)

install(TARGETS polygraph-client
        DESTINATION libexec
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

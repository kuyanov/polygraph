find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)

add_executable(prunner main.cpp)
file(GLOB PRUNNER_SOURCES src/*.cpp)
target_sources(prunner PRIVATE ${PRUNNER_SOURCES})
target_include_directories(prunner PUBLIC src)
target_link_libraries(prunner PRIVATE common ${LIBSBOX_LIBRARY})

install(TARGETS prunner
        DESTINATION libexec
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ WORLD_READ)

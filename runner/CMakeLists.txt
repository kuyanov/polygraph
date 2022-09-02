find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)

add_executable(Runner main.cpp)
file(GLOB RUNNER_SOURCES src/*.cpp)
target_sources(Runner PRIVATE ${RUNNER_SOURCES})
target_compile_definitions(Runner PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/runner/config.json")
target_include_directories(Runner PUBLIC src)
target_link_libraries(Runner PRIVATE Structures Tools ${LIBSBOX_LIBRARY})

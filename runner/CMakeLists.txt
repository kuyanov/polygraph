find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)

add_library(RunnerLib)
file(GLOB RUNNER_SOURCES src/*.cpp)
target_sources(RunnerLib PRIVATE ${RUNNER_SOURCES})
target_compile_definitions(RunnerLib PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/runner/config.json")
target_include_directories(RunnerLib PUBLIC src)
target_link_libraries(RunnerLib PUBLIC Common Structures ${LIBSBOX_LIBRARY})

add_executable(Runner main.cpp)
target_link_libraries(Runner PRIVATE RunnerLib)

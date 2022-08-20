add_executable(TestScheduler test.cpp)
target_compile_definitions(TestScheduler PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/scheduler/config.json")
target_link_libraries(TestScheduler PRIVATE Common gtest_main)

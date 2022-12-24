add_executable(TestScheduler test.cpp)
target_compile_definitions(TestScheduler PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/scheduler/config.json")
target_link_libraries(TestScheduler PRIVATE Structures Tools GTest::gmock_main)

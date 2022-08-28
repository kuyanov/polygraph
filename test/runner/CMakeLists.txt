add_executable(TestRunner test.cpp)
target_compile_definitions(TestRunner PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/runner/config.json")
target_link_libraries(TestRunner PRIVATE Common gtest_main)

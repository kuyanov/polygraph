add_executable(TestRunner test.cpp)
target_link_libraries(TestRunner PRIVATE RunnerLib gtest_main)

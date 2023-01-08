add_executable(test_prunner test.cpp)
target_link_libraries(test_prunner PRIVATE prunner_impl gtest_main)

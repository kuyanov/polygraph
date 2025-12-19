add_executable(test_runner test.cpp)
target_link_libraries(test_runner PRIVATE gtest_main polygraph_impl)

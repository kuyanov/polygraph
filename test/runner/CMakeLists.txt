add_executable(test_runner test.cpp)
target_compile_definitions(test_runner PRIVATE
        RUNNER_CONFIG_PATH="${CMAKE_SOURCE_DIR}/config/runner_conf.json")
target_link_libraries(test_runner PRIVATE runner_impl gtest)

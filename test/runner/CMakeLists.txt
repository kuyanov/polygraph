add_executable(test_prunner test.cpp)
target_compile_definitions(test_prunner PRIVATE
        RUNNER_CONFIG_PATH="${CMAKE_SOURCE_DIR}/runner/runner_conf.json")
target_link_libraries(test_prunner PRIVATE prunner_impl gtest)

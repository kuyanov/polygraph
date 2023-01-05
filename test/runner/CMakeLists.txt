add_executable(test_polyrunner test.cpp)
target_compile_definitions(test_polyrunner PRIVATE
        RUNNER_CONFIG_PATH="${CMAKE_SOURCE_DIR}/runner/runner_conf.json")
target_link_libraries(test_polyrunner PRIVATE polyrunner_impl gtest)

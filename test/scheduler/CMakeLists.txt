add_executable(test_scheduler test.cpp)
target_compile_definitions(test_scheduler PRIVATE
        SCHEDULER_CONFIG_PATH="${CMAKE_SOURCE_DIR}/config/scheduler_conf.json")
target_link_libraries(test_scheduler PRIVATE scheduler_impl gmock)

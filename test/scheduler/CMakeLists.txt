add_executable(test_pscheduler test.cpp)
target_compile_definitions(test_pscheduler PRIVATE
        SCHEDULER_CONFIG_PATH="${CMAKE_SOURCE_DIR}/scheduler/scheduler_conf.json")
target_link_libraries(test_pscheduler PRIVATE pscheduler_impl gmock)

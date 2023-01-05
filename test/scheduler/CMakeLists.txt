add_executable(test_polyscheduler test.cpp)
target_compile_definitions(test_polyscheduler PRIVATE
        SCHEDULER_CONFIG_PATH="${CMAKE_SOURCE_DIR}/scheduler/scheduler_conf.json")
target_link_libraries(test_polyscheduler PRIVATE polyscheduler_impl gmock)

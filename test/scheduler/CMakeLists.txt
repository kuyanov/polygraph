add_executable(TestScheduler main.cpp)
add_compile_definitions(CONFIG_FILE="${CMAKE_SOURCE_DIR}/test/scheduler/config.json")
target_include_directories(TestScheduler PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(TestScheduler PRIVATE Common SchedulerLib Threads::Threads RapidJSON gtest_main)

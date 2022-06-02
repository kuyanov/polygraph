add_executable(TestMasterNode main.cpp)
add_compile_definitions(CONFIG_FILE="${CMAKE_SOURCE_DIR}/test/masternode/config.json")
target_include_directories(TestMasterNode PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(TestMasterNode PRIVATE MasterNodeLib Threads::Threads RapidJSON gtest_main)

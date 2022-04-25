make_test(TestMasterNode test_masternode.cpp)
target_compile_definitions(TestMasterNode PRIVATE
        CONFIG_FILE="${CMAKE_SOURCE_DIR}/test/masternode/config.json")
target_link_libraries(TestMasterNode PRIVATE MasterNodeLib)

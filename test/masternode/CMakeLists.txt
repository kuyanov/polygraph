set(TEST_MASTERNODE_ROOT_DIR ${CMAKE_SOURCE_DIR}/test/masternode)

make_test(TestMasterNode test_masternode.cpp)
target_compile_definitions(TestMasterNode PRIVATE TEST_MASTERNODE_ROOT_DIR="${TEST_MASTERNODE_ROOT_DIR}")
target_link_libraries(TestMasterNode PRIVATE MasterNodeLib)

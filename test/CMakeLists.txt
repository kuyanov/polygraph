function (make_test name)
    add_executable("${name}" ${ARGN})
    target_include_directories("${name}" PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries("${name}" PRIVATE Threads::Threads RapidJSON gtest_main)
    add_test(NAME "${name}" COMMAND "${name}")
endfunction()

make_test(TestMasterNode test_masternode.cpp)
target_link_libraries(TestMasterNode PRIVATE MasterNodeLib)

find_package(Boost REQUIRED COMPONENTS system)

function (make_test name)
    add_executable("${name}" ${ARGN})
    target_include_directories("${name}" PUBLIC ${Boost_INCLUDE_DIRS})
    add_test(NAME "${name}" COMMAND "${name}")
    set_tests_properties("${name}" PROPERTIES TIMEOUT 2)
    target_link_libraries("${name}" PRIVATE gtest_main)
endfunction()

make_test(TestMasterNode test_masternode.cpp)
target_link_libraries(TestMasterNode PRIVATE MasterNodeLib)

# make_test(TestRunner test_runner.cpp)

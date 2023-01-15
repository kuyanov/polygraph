add_executable(polygraph main.cpp)
target_link_libraries(polygraph PRIVATE ${Boost_LIBRARIES} common)

install(TARGETS polygraph RUNTIME DESTINATION ${BIN_DIR})

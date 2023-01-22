add_executable(polygraph main.cpp)
target_link_libraries(polygraph PRIVATE common)

install(TARGETS polygraph DESTINATION bin)

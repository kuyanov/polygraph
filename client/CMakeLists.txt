add_library(pclient_impl)
file(GLOB PCLIENT_SOURCES src/*.cpp)
target_sources(pclient_impl PRIVATE ${PCLIENT_SOURCES})
target_include_directories(pclient_impl PUBLIC src)
target_link_libraries(pclient_impl PUBLIC common)

add_executable(pclient main.cpp)
target_link_libraries(pclient PRIVATE pclient_impl)

install(TARGETS pclient RUNTIME DESTINATION ${EXEC_DIR})

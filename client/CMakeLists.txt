add_library(client_impl)
file(GLOB CLIENT_SOURCES src/*.cpp)
target_sources(client_impl PRIVATE ${CLIENT_SOURCES})
target_include_directories(client_impl PUBLIC src)
target_link_libraries(client_impl PUBLIC common)

add_executable(client main.cpp)
target_link_libraries(client PRIVATE ${Boost_LIBRARIES} client_impl)

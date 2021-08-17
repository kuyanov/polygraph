find_package(Boost REQUIRED)
add_executable(Worker main.cpp)
target_include_directories(Worker PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(Worker PRIVATE ${Boost_LIBRARY} RapidJSON)

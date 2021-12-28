find_package(Boost REQUIRED COMPONENTS system)

add_executable(Runner main.cpp)
target_include_directories(Runner PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(Runner PRIVATE RapidJSON)

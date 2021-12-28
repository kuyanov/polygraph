find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

add_executable(Runner main.cpp)
target_include_directories(Runner PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(Runner PRIVATE Threads::Threads ${Boost_LIBRARY} RapidJSON)

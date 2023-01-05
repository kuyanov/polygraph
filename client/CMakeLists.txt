add_library(polyrun_impl)
file(GLOB POLYRUN_SOURCES src/*.cpp)
target_sources(polyrun_impl PRIVATE ${POLYRUN_SOURCES})
target_include_directories(polyrun_impl PUBLIC src)
target_link_libraries(polyrun_impl PUBLIC common)

add_executable(polyrun main.cpp)
target_link_libraries(polyrun PRIVATE ${Boost_LIBRARIES} polyrun_impl)

install(TARGETS polyrun)

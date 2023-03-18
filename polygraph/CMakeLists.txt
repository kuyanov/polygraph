add_executable(polygraph main.cpp)
file(GLOB POLYGRAPH_SOURCES src/*.cpp)
target_sources(polygraph PRIVATE ${POLYGRAPH_SOURCES})
target_include_directories(polygraph PUBLIC src)
target_link_libraries(polygraph PRIVATE common)

install(TARGETS polygraph DESTINATION bin)

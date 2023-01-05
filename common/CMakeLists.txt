add_library(common)
file(GLOB COMMON_SOURCES *.cpp serialization/*.cpp)
target_sources(common PRIVATE ${COMMON_SOURCES})
target_include_directories(common PUBLIC .)

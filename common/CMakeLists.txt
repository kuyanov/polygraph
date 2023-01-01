add_library(Common)
file(GLOB COMMON_SOURCES *.cpp)
target_sources(Common PRIVATE ${COMMON_SOURCES})
target_include_directories(Common PUBLIC .)

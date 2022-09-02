add_library(Structures)
file(GLOB STRUCTURES_SOURCES *.cpp)
target_sources(Structures PRIVATE ${STRUCTURES_SOURCES})
target_include_directories(Structures PUBLIC .)

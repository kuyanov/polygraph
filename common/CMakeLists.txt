add_library(Common)
file(GLOB COMMON_SOURCES *.cpp)
target_sources(Common PRIVATE ${COMMON_SOURCES})
target_compile_definitions(Common PUBLIC
        SCHEMA_DIR="${CMAKE_SOURCE_DIR}/schema"
        ROOT_DIR="${CMAKE_SOURCE_DIR}/shared")
target_include_directories(Common PUBLIC .)

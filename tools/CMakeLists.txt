add_library(Tools)
file(GLOB COMMON_SOURCES *.cpp)
target_sources(Tools PRIVATE ${COMMON_SOURCES})
target_compile_definitions(Tools PUBLIC
        SCHEMA_DIR="${CMAKE_SOURCE_DIR}/schema"
        SANDBOX_DIR="${CMAKE_SOURCE_DIR}/sandbox")
target_include_directories(Tools PUBLIC .)

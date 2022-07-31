add_library(Common)
file(GLOB COMMON_SOURCES *.cpp)
target_sources(Common PRIVATE ${COMMON_SOURCES})
target_compile_definitions(Common PUBLIC
        SCHEMA_DIR="${CMAKE_SOURCE_DIR}/schema"
        SANDBOX_DIR="${CMAKE_SOURCE_DIR}/sandbox"
        USER_DIR="${CMAKE_SOURCE_DIR}/user")
target_include_directories(Common PUBLIC .)
target_link_libraries(Common PRIVATE RapidJSON)

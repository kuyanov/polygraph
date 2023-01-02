add_library(ClientLib)
file(GLOB CLIENT_SOURCES src/*.cpp)
target_sources(ClientLib PRIVATE ${CLIENT_SOURCES})
target_compile_definitions(ClientLib PUBLIC CONFIG_FILE="${CMAKE_SOURCE_DIR}/client/config.json")
target_include_directories(ClientLib PUBLIC src)
target_link_libraries(ClientLib PUBLIC Common Structures)

add_executable(Client main.cpp)
target_link_libraries(Client PRIVATE ClientLib)

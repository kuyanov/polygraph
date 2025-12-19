cmake_minimum_required(VERSION 3.5)
project(polygraph)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_TSAN "-g -fsanitize=thread")
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

find_package(Boost REQUIRED COMPONENTS program_options)
find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)
include(cmake/rapidjson.cmake)
include(cmake/uWebSockets.cmake)

file(GLOB POLYGRAPH_SOURCES src/*.cpp)
add_library(polygraph_impl STATIC ${POLYGRAPH_SOURCES})
target_compile_definitions(polygraph_impl PUBLIC
    CONF_PATH="/etc/${PROJECT_NAME}/config.json"
    SCHEMA_DIR="${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/schema"
    LOG_PATH="/var/log/${PROJECT_NAME}.log"
    CONTAINERS_DIR="/var/${PROJECT_NAME}/containers"
    RUN_DIR="/var/run/${PROJECT_NAME}"
)
target_include_directories(polygraph_impl PUBLIC src ${Boost_INCLUDE_DIR} ${rapidjson_SOURCE_DIR}/include)
target_link_libraries(polygraph_impl PRIVATE ${Boost_LIBRARIES} ${LIBSBOX_LIBRARY} uWebSockets)

add_executable(polygraph main.cpp)
target_link_libraries(polygraph PRIVATE polygraph_impl)

include(cmake/googletest.cmake)
add_subdirectory(test)

install(DIRECTORY schema DESTINATION "share/${PROJECT_NAME}")
install(FILES config.json DESTINATION "/etc/${PROJECT_NAME}")
install(TARGETS polygraph DESTINATION bin)

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
include(cmake/rapidjson.cmake)
include(cmake/uWebSockets.cmake)

add_compile_definitions(
        DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}"
        EXEC_DIR="${CMAKE_INSTALL_PREFIX}/libexec"
        LOG_DIR="/var/log/${PROJECT_NAME}"
        RUN_DIR="/var/run/${PROJECT_NAME}"
        VAR_DIR="/var/${PROJECT_NAME}"
)

include_directories(${Boost_INCLUDE_DIR} ${rapidjson_SOURCE_DIR}/include)
link_libraries(${Boost_LIBRARIES})

add_subdirectory(common)
add_subdirectory(polygraph)
add_subdirectory(polygraph-client)
add_subdirectory(polygraph-runner)
add_subdirectory(polygraph-scheduler)
if (ENABLE_TESTING)
    include(cmake/googletest.cmake)
    add_subdirectory(test)
endif ()

install(DIRECTORY schema DESTINATION share/${PROJECT_NAME})

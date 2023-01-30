cmake_minimum_required(VERSION 3.16)
project(polygraph)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address")
set(CMAKE_CXX_FLAGS_TSAN "-g -fsanitize=thread")
set(CMAKE_EXPORT_NO_PACKAGE_REGISTRY TRUE)
set(FETCHCONTENT_QUIET FALSE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (NOT USER_DIR)
    set(USER_DIR /home/polygraph)
endif ()

find_package(Boost REQUIRED COMPONENTS program_options)

FetchContent_Declare(
        rapidjson
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_SUBMODULES ""
)
FetchContent_Declare(
        uWebSockets
        GIT_REPOSITORY https://github.com/uNetworking/uWebSockets.git
        GIT_TAG v20.22.0
        GIT_SUBMODULES uSockets
        GIT_SUBMODULES_RECURSE FALSE
)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)

FetchContent_GetProperties(rapidjson)
if (NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif ()

add_compile_definitions(
        CONF_DIR="/etc/${PROJECT_NAME}"
        DATA_DIR="${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}"
        EXEC_DIR="${CMAKE_INSTALL_PREFIX}/libexec"
        LOG_DIR="/var/log/${PROJECT_NAME}"
        RUN_DIR="/var/run/${PROJECT_NAME}"
        VAR_DIR="/var/${PROJECT_NAME}"
)

include_directories(${Boost_INCLUDE_DIR} ${rapidjson_SOURCE_DIR}/include)
link_libraries(${Boost_LIBRARIES})

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(frontend)
if (NOT DISABLE_RUNNER)
    add_subdirectory(runner)
endif ()
if (NOT DISABLE_SCHEDULER)
    add_subdirectory(scheduler)
endif ()
if (NOT DISABLE_TESTING)
    add_subdirectory(test)
endif ()

macro(install_symlink file_path symlink_path)
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${file_path} ${symlink_path})")
    install(CODE "message(\"-- Created symlink: ${symlink_path} -> ${file_path}\")")
endmacro()

install(FILES config.json DESTINATION /etc/${PROJECT_NAME})
install(DIRECTORY schema DESTINATION share/${PROJECT_NAME})
install(DIRECTORY DESTINATION /var/${PROJECT_NAME}/user DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_WRITE GROUP_EXECUTE
        WORLD_READ WORLD_WRITE WORLD_EXECUTE)

install_symlink(/var/${PROJECT_NAME}/user ${USER_DIR})

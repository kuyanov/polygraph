find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)

add_library(runner_impl)
file(GLOB RUNNER_SOURCES src/*.cpp)
target_sources(runner_impl PRIVATE ${RUNNER_SOURCES})
target_include_directories(runner_impl PUBLIC src)
target_link_libraries(runner_impl PUBLIC common ${LIBSBOX_LIBRARY})

add_executable(runner main.cpp)
target_compile_definitions(runner PRIVATE
        DEFAULT_CONFIG_PATH="${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}/runner_conf.json")
target_link_libraries(runner PRIVATE ${Boost_LIBRARIES} runner_impl)

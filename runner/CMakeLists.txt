find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)

add_library(polyrunner_impl)
file(GLOB POLYRUNNER_SOURCES src/*.cpp)
target_sources(polyrunner_impl PRIVATE ${POLYRUNNER_SOURCES})
target_include_directories(polyrunner_impl PUBLIC src)
target_link_libraries(polyrunner_impl PUBLIC common ${LIBSBOX_LIBRARY})

add_executable(polyrunner main.cpp)
target_compile_definitions(polyrunner PRIVATE
        DEFAULT_CONFIG_PATH="${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}/runner_conf.json")
target_link_libraries(polyrunner PRIVATE ${Boost_LIBRARIES} polyrunner_impl)

install(TARGETS polyrunner)
install(FILES runner_conf.json
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME})

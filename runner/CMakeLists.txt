find_library(LIBSBOX_LIBRARY libsbox.a REQUIRED)

add_library(prunner_impl)
file(GLOB PRUNNER_SOURCES src/*.cpp)
target_sources(prunner_impl PRIVATE ${PRUNNER_SOURCES})
target_include_directories(prunner_impl PUBLIC src)
target_link_libraries(prunner_impl PUBLIC common ${LIBSBOX_LIBRARY})

add_executable(prunner main.cpp)
target_link_libraries(prunner PRIVATE ${Boost_LIBRARIES} prunner_impl)

install(TARGETS prunner RUNTIME DESTINATION ${EXEC_DIR})

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (ENABLE_RUNNER)
    add_subdirectory(runner)
endif ()
if (ENABLE_SCHEDULER)
    add_subdirectory(scheduler)
endif ()

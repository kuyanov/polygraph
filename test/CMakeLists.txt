set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (NOT DISABLE_RUNNER)
    add_subdirectory(runner)
endif ()
if (NOT DISABLE_SCHEDULER)
    add_subdirectory(scheduler)
endif ()

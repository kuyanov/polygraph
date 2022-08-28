include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (ENABLE_RUNNER)
    add_subdirectory(runner)
endif ()
if (ENABLE_SCHEDULER)
    add_subdirectory(scheduler)
endif ()

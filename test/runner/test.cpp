#include "gtest/gtest.h"
#include "check.h"
#include "config.h"

TEST(Network, Reconnect) {
    long long start_time, end_time;
    {
        auto session = server.Accept();
        start_time = Timestamp();
    }
    {
        auto session = server.Accept();
        end_time = Timestamp();
    }
    CheckDuration(end_time - start_time, Config::Get().reconnect_interval_ms);
}

TEST(Execution, Sleep) {
    InitContainer();
    auto response = SendTask({.argv = {"sleep", "1"}});
    CheckExitedNormally(response);
    CheckDuration(response.result->time_usage_ms, 0);
    CheckDuration(response.result->wall_time_usage_ms, 1000);
}

TEST(Execution, Stdio) {
    InitContainer();
    AddFile("input", "hello world");
    AddFile("output", "");
    AddFile("script", "#!/bin/bash\ncat");
    auto response1 =
        SendTask({.argv = {"bash", "-c", "./script <input >output"}, .limits = {.max_threads = 3}});
    CheckExitedNormally(response1);
    ASSERT_EQ(ReadFile("output"), "hello world");
    AddFile("error", "");
    auto response2 = SendTask({.argv = {"bash", "-c", "echo test 2>error 1>&2"}});
    CheckExitedNormally(response2);
    ASSERT_EQ(ReadFile("error"), "test\n");
}

TEST(Execution, Permissions) {
    InitContainer();
    AddFile("input", "", 3);
    auto response1 = SendTask({.argv = {"bash", "-c", "cat <input"}, .limits = {.max_threads = 2}});
    ASSERT_TRUE(response1.result.has_value());
    ASSERT_EQ(response1.result->exit_code, 1);
    AddFile("output", "", 5);
    auto response2 = SendTask({.argv = {"bash", "-c", "echo test >output"}});
    ASSERT_TRUE(response2.result.has_value());
    ASSERT_EQ(response2.result->exit_code, 1);
    AddFile("script", "#!/bin/bash\ncat", 6);
    auto response3 = SendTask({.argv = {"bash", "-c", "./script"}, .limits = {.max_threads = 3}});
    ASSERT_TRUE(response3.result.has_value());
    ASSERT_EQ(response3.result->exit_code, 126);
}

TEST(Execution, Environment) {
    InitContainer();
    auto response = SendTask({.argv = {"bash", "-c", "exit $CODE"}, .env = {"CODE=57"}});
    ASSERT_TRUE(response.result.has_value());
    ASSERT_EQ(response.result->exit_code, 57);
}

TEST(Execution, TimeLimit) {
    InitContainer();
    auto response = SendTask(
        {.argv = {"bash", "-c", "while true; do :; done"}, .limits = {.time_limit_ms = 1000}});
    ASSERT_TRUE(response.result.has_value());
    ASSERT_TRUE(response.result->time_limit_exceeded);
    CheckDuration(response.result->time_usage_ms, 1000);
}

TEST(Execution, WallTimeLimit) {
    InitContainer();
    auto response = SendTask({.argv = {"sleep", "2"}, .limits = {.wall_time_limit_ms = 1000}});
    ASSERT_TRUE(response.result.has_value());
    ASSERT_TRUE(response.result->wall_time_limit_exceeded);
    CheckDuration(response.result->wall_time_usage_ms, 1000);
}

TEST(Execution, MemoryLimit) {
    InitContainer();
    auto response = SendTask({.argv = {"bash", "-c", "a=(0); while true; do a+=$a; done"},
                              .limits = {.memory_limit_kb = 1024}});
    ASSERT_TRUE(response.result.has_value());
    ASSERT_TRUE(response.result->memory_limit_exceeded);
}

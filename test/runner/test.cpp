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

TEST(Execution, Empty) {
    auto response = SendTasks({});
    ASSERT_TRUE(!response.has_error);
    ASSERT_TRUE(response.results.empty());
}

TEST(Execution, Sleep) {
    InitContainer();
    auto response = SendTasks({{.argv = {"sleep", "1"}}});
    CheckAllExitedNormally(response);
    CheckDuration(response.results[0].time_usage_ms, 0);
    CheckDuration(response.results[0].wall_time_usage_ms, 1000);
}

TEST(Execution, Stdio) {
    InitContainer();
    AddFile("input", "Hello, world!");
    AddFile("output", "");
    auto response = SendTasks({{.argv = {"cat"}, .stdin_ = "input", .stdout_ = "output"}});
    CheckAllExitedNormally(response);
    ASSERT_EQ(ReadFile("output"), "Hello, world!");
}

TEST(Execution, Stderr) {
    InitContainer();
    AddFile("error", "");
    auto response = SendTasks({{.argv = {"bash", "-c", "echo test 1>&2"}, .stderr_ = "error"}});
    CheckAllExitedNormally(response);
    ASSERT_EQ(ReadFile("error"), "test\n");
}

TEST(Execution, StderrRedirect) {
    InitContainer();
    AddFile("output", "");
    auto response = SendTasks({{.argv = {"bash", "-c", "echo test 1>&2"}, .stdout_ = "output"}});
    CheckAllExitedNormally(response);
    ASSERT_EQ(ReadFile("output"), "test\n");
}

TEST(Execution, Binds) {
    InitContainer();
    InitUserDir();
    AddUserFile("input.txt", "Hello, world!");
    AddUserFile("output.txt", "");
    AddUserFile("script.sh", "#!/bin/bash\ncat\n", true);
    auto response = SendTasks({{.argv = {"./script.sh"},
                                .binds = {{"input", "input.txt", false},
                                          {"output", "output.txt", true},
                                          {"script.sh", "script.sh", false}},
                                .stdin_ = "input",
                                .stdout_ = "output",
                                .max_threads = 2}});
    CheckAllExitedNormally(response);
    ASSERT_EQ(ReadUserFile("output.txt"), "Hello, world!");
}

TEST(Execution, WriteForbidden) {
    InitContainer();
    InitUserDir();
    AddUserFile("protected.txt", "");
    auto response = SendTasks({{.argv = {"bash", "-c", "echo rubbish >protected"},
                                .binds = {{"protected", "protected.txt", false}}}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_EQ(response.results[0].exit_code, 1);
}

TEST(Execution, Environment) {
    InitContainer();
    auto response = SendTasks({{.argv = {"bash", "-c", "exit $CODE"}, .env = {"CODE=57"}}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_EQ(response.results[0].exit_code, 57);
}

TEST(Execution, TimeLimit) {
    InitContainer();
    auto response =
        SendTasks({{.argv = {"bash", "-c", "while true; do :; done"}, .time_limit_ms = 1000}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_TRUE(response.results[0].time_limit_exceeded);
    CheckDuration(response.results[0].time_usage_ms, 1000);
}

TEST(Execution, WallTimeLimit) {
    InitContainer();
    auto response = SendTasks({{.argv = {"sleep", "2"}, .wall_time_limit_ms = 1000}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_TRUE(response.results[0].wall_time_limit_exceeded);
    CheckDuration(response.results[0].wall_time_usage_ms, 1000);
}

TEST(Execution, MemoryLimit) {
    InitContainer();
    auto response = SendTasks(
        {{.argv = {"bash", "-c", "a=(0); while true; do a+=$a; done"}, .memory_limit_kb = 1024}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_TRUE(response.results[0].memory_limit_exceeded);
}

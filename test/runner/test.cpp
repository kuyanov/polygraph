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
    auto container_path = CreateContainer();
    auto response =
        SendRunRequest({.binds = {{".", container_path.string(), false}}, .argv = {"sleep", "1"}});
    CheckExitedNormally(response);
    CheckDuration(response.status->time_usage_ms, 0);
    CheckDuration(response.status->wall_time_usage_ms, 1000);
}

TEST(Execution, Mounts) {
    auto container_path = CreateContainer();
    auto input_path = paths::kTestDir / "input";
    auto output_path = paths::kTestDir / "output";
    auto script_path = paths::kTestDir / "script";
    CreateFile(input_path, "hello world");
    CreateFile(output_path, "");
    CreateFile(script_path, "#!/bin/bash\ncat");
    auto response1 = SendRunRequest({.binds = {{".", container_path.string(), false},
                                               {"input", input_path.string(), true},
                                               {"output", output_path.string(), false},
                                               {"script", script_path.string(), true}},
                                     .argv = {"bash", "-c", "./script <input >output"},
                                     .constraints = {.max_threads = 3}});
    CheckExitedNormally(response1);
    ASSERT_EQ(ReadFile(output_path), "hello world");
    auto error_path = paths::kTestDir / "error";
    CreateFile(error_path, "");
    auto response2 = SendRunRequest(
        {.binds = {{".", container_path.string(), false}, {"error", error_path.string(), false}},
         .argv = {"bash", "-c", "echo test 2>error 1>&2"}});
    CheckExitedNormally(response2);
    ASSERT_EQ(ReadFile(error_path), "test\n");
    auto response3 = SendRunRequest(
        {.binds = {{".", container_path.string(), false}, {"error", error_path.string(), true}},
         .argv = {"bash", "-c", "echo test 2>error 1>&2"}});
    ASSERT_TRUE(response3.status.has_value());
    ASSERT_EQ(response3.status->exit_code, 1);
}

TEST(Execution, Permissions) {
    auto container_path = CreateContainer();
    auto input_path = paths::kTestDir / "input";
    CreateFile(input_path, "", 3);
    auto response1 = SendRunRequest(
        {.binds = {{".", container_path.string(), false}, {"input", input_path.string(), true}},
         .argv = {"bash", "-c", "cat <input"},
         .constraints = {.max_threads = 2}});
    ASSERT_TRUE(response1.status.has_value());
    ASSERT_EQ(response1.status->exit_code, 1);
    auto output_path = paths::kTestDir / "output";
    CreateFile(output_path, "", 5);
    auto response2 = SendRunRequest(
        {.binds = {{".", container_path.string(), false}, {"output", output_path.string(), false}},
         .argv = {"bash", "-c", "echo test >output"}});
    ASSERT_TRUE(response2.status.has_value());
    ASSERT_EQ(response2.status->exit_code, 1);
    auto script_path = paths::kTestDir / "script";
    CreateFile(script_path, "#!/bin/bash\necho test", 6);
    auto response3 = SendRunRequest(
        {.binds = {{".", container_path.string(), false}, {"script", script_path.string(), true}},
         .argv = {"bash", "-c", "./script"},
         .constraints = {.max_threads = 3}});
    ASSERT_TRUE(response3.status.has_value());
    ASSERT_EQ(response3.status->exit_code, 126);
}

TEST(Execution, Environment) {
    auto container_path = CreateContainer();
    auto response = SendRunRequest({.binds = {{".", container_path.string(), false}},
                                    .argv = {"bash", "-c", "exit $CODE"},
                                    .env = {"CODE=57"}});
    ASSERT_TRUE(response.status.has_value());
    ASSERT_EQ(response.status->exit_code, 57);
}

TEST(Execution, TimeLimit) {
    auto container_path = CreateContainer();
    auto response = SendRunRequest({.binds = {{".", container_path.string(), false}},
                                    .argv = {"bash", "-c", "while true; do :; done"},
                                    .constraints = {.time_limit_ms = 1000}});
    ASSERT_TRUE(response.status.has_value());
    ASSERT_TRUE(response.status->time_limit_exceeded);
    CheckDuration(response.status->time_usage_ms, 1000);
}

TEST(Execution, WallTimeLimit) {
    auto container_path = CreateContainer();
    auto response = SendRunRequest({.binds = {{".", container_path.string(), false}},
                                    .argv = {"sleep", "2"},
                                    .constraints = {.wall_time_limit_ms = 1000}});
    ASSERT_TRUE(response.status.has_value());
    ASSERT_TRUE(response.status->wall_time_limit_exceeded);
    CheckDuration(response.status->wall_time_usage_ms, 1000);
}

TEST(Execution, MemoryLimit) {
    auto container_path = CreateContainer();
    auto response = SendRunRequest({.binds = {{".", container_path.string(), false}},
                                    .argv = {"bash", "-c", "a=(0); while true; do a+=$a; done"},
                                    .constraints = {.memory_limit_kb = 1024}});
    ASSERT_TRUE(response.status.has_value());
    ASSERT_TRUE(response.status->memory_limit_exceeded);
}

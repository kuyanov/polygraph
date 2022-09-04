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
    auto response = SendTasks({{.argv = {"sleep", "1"}}});
    CheckAllExitedNormally(response);
    CheckDuration(response.results[0].time_usage_ms, 0);
    CheckDuration(response.results[0].wall_time_usage_ms, 1000);
}

TEST(Execution, Environment) {
    auto response = SendTasks({{.argv = {"bash", "-c", "exit $CODE"}, .env = {"CODE=57"}}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_EQ(response.results[0].exit_code, 57);
}

TEST(Execution, TimeLimit) {
    auto response =
        SendTasks({{.argv = {"bash", "-c", "while true; do :; done"}, .time_limit_ms = 1000}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_TRUE(response.results[0].time_limit_exceeded);
    CheckDuration(response.results[0].time_usage_ms, 1000);
}

TEST(Execution, WallTimeLimit) {
    auto response = SendTasks({{.argv = {"sleep", "2"}, .wall_time_limit_ms = 1000}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_TRUE(response.results[0].wall_time_limit_exceeded);
    CheckDuration(response.results[0].wall_time_usage_ms, 1000);
}

TEST(Execution, MemoryLimit) {
    auto response = SendTasks(
        {{.argv = {"bash", "-c", "a=(0); while true; do a+=$a; done"}, .memory_limit_kb = 1024}});
    ASSERT_TRUE(!response.has_error);
    ASSERT_TRUE(response.results[0].memory_limit_exceeded);
}

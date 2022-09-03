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
    auto [response, duration] = SendTasks({});
    ASSERT_TRUE(!response.has_error && response.results.empty());
    CheckDuration(duration, 0);
}

TEST(Execution, Sleep) {
    auto [response, duration] = SendTasks({{{"sleep", "1"}}});
    CheckAllExitedNormally(response);
    CheckDuration(duration, 1000);
}

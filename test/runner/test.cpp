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
    CheckTimeDelta(start_time, end_time, Config::Get().reconnect_interval_ms);
}

TEST(Execution, Empty) {
    auto response = SendTasks({});
    ASSERT_TRUE(!response.has_error && response.results.empty());
}

TEST(Execution, Sleep) {
    auto start_time = Timestamp();
    auto response = SendTasks({{{"sleep", "1"}}});
    auto end_time = Timestamp();
    CheckAllExitedNormally(response);
    CheckTimeDelta(start_time, end_time, 1000);
}

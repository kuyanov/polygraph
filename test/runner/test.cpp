#include <chrono>
#include <string>

#include "gtest/gtest.h"
#include "config.h"
#include "net.h"

static WebsocketServer server(Config::Get().scheduler_host, Config::Get().scheduler_port);

long long Timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

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
    auto error = end_time - start_time - Config::Get().reconnect_interval_ms;
    ASSERT_TRUE(error >= 0 && error < 100);
}

TEST(Execution, Empty) {
    auto session = server.Accept();
    session.Write("{\"tasks\":[],\"container\":\"test\"}");
    auto message = session.Read();
}

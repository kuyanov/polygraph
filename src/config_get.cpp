#include <iostream>

#include "config.h"
#include "config_get.h"

void ConfigGet(const ConfigGetOptions &options) {
    std::cout << "host                         : " << Config::Get().host << std::endl;
    std::cout << "port                         : " << Config::Get().port << std::endl;
    std::cout << "runner_reconnect_interval_ms : " << Config::Get().runner_reconnect_interval_ms
              << std::endl;
    std::cout << "runner_timer_interval_ms     : " << Config::Get().runner_timer_interval_ms
              << std::endl;
    std::cout << "scheduler_max_payload_length : " << Config::Get().scheduler_max_payload_length
              << std::endl;
    std::cout << "scheduler_idle_timeout_s     : " << Config::Get().scheduler_idle_timeout_s
              << std::endl;
}

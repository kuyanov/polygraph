#pragma once

#include <optional>
#include <string>
#include <boost/program_options.hpp>

#include "config.h"

namespace po = boost::program_options;

class ConfigOptions {
public:
    std::optional<unsigned int> runner_reconnect_interval_ms;
    std::optional<std::string> scheduler_host;
    std::optional<int> scheduler_port;
    std::optional<unsigned int> scheduler_max_payload_length;
    std::optional<unsigned short> scheduler_idle_timeout;

    static ConfigOptions &Get() {
        static ConfigOptions options;
        return options;
    }

    void Init(int argc, char **argv) {
        po::options_description desc;
        desc.add_options()("runner-reconnect-interval-ms", po::value<unsigned int>());
        desc.add_options()("scheduler-host", po::value<std::string>());
        desc.add_options()("scheduler-port", po::value<int>());
        desc.add_options()("scheduler-max-payload-length", po::value<unsigned int>());
        desc.add_options()("scheduler-idle-timeout", po::value<unsigned short>());
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("runner-reconnect-interval-ms")) {
            runner_reconnect_interval_ms = vm["runner-reconnect-interval-ms"].as<unsigned int>();
        }
        if (vm.count("scheduler-host")) {
            scheduler_host = vm["scheduler-host"].as<std::string>();
        }
        if (vm.count("scheduler-port")) {
            scheduler_port = vm["scheduler-port"].as<int>();
        }
        if (vm.count("scheduler-max-payload-length")) {
            scheduler_max_payload_length = vm["scheduler-max-payload-length"].as<unsigned int>();
        }
        if (vm.count("scheduler-idle-timeout")) {
            scheduler_idle_timeout = vm["scheduler-idle-timeout"].as<unsigned short>();
        }
    }

    void SetConfig() {
        if (runner_reconnect_interval_ms.has_value()) {
            Config::Get().runner_reconnect_interval_ms = runner_reconnect_interval_ms.value();
        }
        if (scheduler_host.has_value()) {
            Config::Get().scheduler_host = scheduler_host.value();
        }
        if (scheduler_port.has_value()) {
            Config::Get().scheduler_port = scheduler_port.value();
        }
        if (scheduler_max_payload_length.has_value()) {
            Config::Get().scheduler_max_payload_length = scheduler_max_payload_length.value();
        }
        if (scheduler_idle_timeout.has_value()) {
            Config::Get().scheduler_idle_timeout = scheduler_idle_timeout.value();
        }
    }
};

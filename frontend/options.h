#pragma once

#include <optional>
#include <string>
#include <boost/program_options.hpp>

#include "config.h"

namespace po = boost::program_options;

class ConfigOptions {
public:
    std::optional<std::string> host;
    std::optional<int> port;

    static ConfigOptions &Get() {
        static ConfigOptions options;
        return options;
    }

    void Init(int argc, char **argv) {
        po::options_description desc;
        desc.add_options()("host", po::value<std::string>());
        desc.add_options()("port", po::value<int>());
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("host")) {
            host = vm["host"].as<std::string>();
        }
        if (vm.count("port")) {
            port = vm["port"].as<int>();
        }
    }

    void SetConfig() {
        if (host.has_value()) {
            Config::Get().scheduler_host = host.value();
        }
        if (port.has_value()) {
            Config::Get().scheduler_port = port.value();
        }
    }
};

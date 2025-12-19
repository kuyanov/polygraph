#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/program_options.hpp>

#include "config.h"

namespace po = boost::program_options;

class ConfigSetOptions {
public:
    po::options_description desc{"Options"};

    void HelpMessage() const {
        std::cerr << "Update polygraph configuration." << std::endl;
        std::cerr << std::endl;
        std::cerr << "Usage:  " << "polygraph config set [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        desc.add_options()("host", po::value<std::string>(&Config::Get().host), "scheduler host");
        desc.add_options()("port", po::value<int>(&Config::Get().port), "scheduler port");
        desc.add_options()(
            "runner-reconnect-interval-ms",
            po::value<int>(&Config::Get().runner_reconnect_interval_ms),
            "duration between consecutive runner reconnect attempts to the scheduler");
        desc.add_options()("runner-timer-interval-ms",
                           po::value<int>(&Config::Get().runner_timer_interval_ms),
                           "duration between consecutive timer cycles to control task time limit");
        desc.add_options()("scheduler-max-payload-length",
                           po::value<int>(&Config::Get().scheduler_max_payload_length),
                           "maximum payload length for scheduler");
        desc.add_options()("scheduler-idle-timeout-s",
                           po::value<int>(&Config::Get().scheduler_idle_timeout_s),
                           "time limit after which network connections are automatically closed");
        po::variables_map vm;
        try {
            po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
            po::notify(vm);
        } catch (const po::error &e) {
            std::cerr << "Couldn't parse command line arguments:" << std::endl;
            std::cerr << e.what() << std::endl;
            std::cerr << std::endl;
            HelpMessage();
        }
        if (vm.count("help")) {
            HelpMessage();
        }
    }
};

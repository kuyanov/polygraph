#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "environment.h"

namespace po = boost::program_options;

class RunnerStartOptions {
public:
    po::options_description desc{"Options"};
    std::string host;
    int port;
    int num;
    std::string partition;
    unsigned int reconnect_interval_ms;

    void HelpMessage() const {
        std::cerr << "Usage:  "
                  << "polygraph runner start [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        std::string default_host = GetEnvOr("POLYGRAPH_HOST", "127.0.0.1");
        desc.add_options()("host", po::value<std::string>(&host)->default_value(default_host),
                           "polygraph host (default taken from POLYGRAPH_HOST)");
        int default_port = atoi(GetEnvOr("POLYGRAPH_PORT", "3000").c_str());
        desc.add_options()("port", po::value<int>(&port)->default_value(default_port),
                           "polygraph port (default taken from POLYGRAPH_PORT)");
        desc.add_options()("num", po::value<int>(&num)->default_value(1),
                           "number of runners to start");
        desc.add_options()("partition", po::value<std::string>(&partition)->default_value("all"),
                           "partition to subscribe to");
        unsigned int default_reconnect_interval_ms =
            atoi(GetEnvOr("POLYGRAPH_RUNNER_RECONNECT_INTERVAL_MS", "1000").c_str());
        desc.add_options()("reconnect-interval",
                           po::value<unsigned int>(&reconnect_interval_ms)
                               ->default_value(default_reconnect_interval_ms),
                           "time interval before reconnecting to scheduler (default taken from "
                           "POLYGRAPH_RUNNER_RECONNECT_INTERVAL_MS)");
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

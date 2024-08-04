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
        std::cerr << "Usage:  " << "polygraph runner start [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        desc.add_options()("host", po::value<std::string>(&host)->default_value("127.0.0.1"),
                           "polygraph host");
        desc.add_options()("port", po::value<int>(&port)->default_value(3000), "polygraph port");
        desc.add_options()("num", po::value<int>(&num)->default_value(1),
                           "number of runners to start");
        desc.add_options()("partition", po::value<std::string>(&partition)->default_value("all"),
                           "partition to subscribe to");
        desc.add_options()("reconnect-interval",
                           po::value<unsigned int>(&reconnect_interval_ms)->default_value(1000),
                           "time interval (ms) before reconnecting to scheduler");
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

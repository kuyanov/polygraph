#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/program_options.hpp>

#include "environment.h"

namespace po = boost::program_options;

class StartOptions {
public:
    po::options_description desc{"Options"};
    int port;
    unsigned int max_payload_length;
    unsigned short idle_timeout;

    void HelpMessage() const {
        std::cerr << "Usage:  "
                  << "polygraph start [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        int default_port = atoi(GetEnvOr("POLYGRAPH_PORT", "3000").c_str());
        desc.add_options()("port", po::value<int>(&port)->default_value(default_port),
                           "polygraph port (default taken from POLYGRAPH_PORT)");
        unsigned int default_mplen = atoi(GetEnvOr("POLYGRAPH_SCHEDULER_MPLEN", "1048576").c_str());
        desc.add_options()(
            "mplen", po::value<unsigned int>(&max_payload_length)->default_value(default_mplen),
            "max payload length, in bytes (default taken from POLYGRAPH_SCHEDULER_MPLEN)");
        unsigned short default_idle_timeout =
            atoi(GetEnvOr("POLYGRAPH_SCHEDULER_IDLE_TIMEOUT_S", "60").c_str());
        desc.add_options()(
            "idle-timeout",
            po::value<unsigned short>(&idle_timeout)->default_value(default_idle_timeout),
            "time interval for pings (default taken from POLYGRAPH_SCHEDULER_IDLE_TIMEOUT_S)");
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

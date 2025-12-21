#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class StartOptions {
public:
    po::options_description desc{"Options"};
    int num_runners;
    std::string partition;

    void HelpMessage() const {
        std::cerr << "Start polygraph service." << std::endl;
        std::cerr << "Note: no runners are started by default. Specify the --runners option or "
                     "start them separately via 'polygraph runner start'."
                  << std::endl;
        std::cerr << std::endl;
        std::cerr << "Usage:  " << "polygraph start [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        desc.add_options()("runners", po::value<int>(&num_runners)->default_value(0),
                           "number of runners to start");
        desc.add_options()("partition", po::value<std::string>(&partition)->default_value("all"),
                           "partition to subscribe runners to");
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

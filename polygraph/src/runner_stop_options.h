#pragma once

#pragma once

#include <cstdlib>
#include <iostream>
#include <vector>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class RunnerStopOptions {
public:
    po::options_description desc{"Options"};
    std::vector<int> ids;

    void HelpMessage() const {
        std::cerr << "Usage:  " << "polygraph runner stop [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        desc.add_options()("ids", po::value<std::vector<int>>(&ids)->multitoken(),
                           "list of runner ids to stop, empty means all");
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

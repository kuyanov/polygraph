#pragma once

#include <cstdlib>
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class StartOptions {
public:
    po::options_description desc{"Options"};

    void HelpMessage() const {
        std::cerr << "Start polygraph service." << std::endl;
        std::cerr << "Note: this command does not start any runners. Run 'polygraph runner start' "
                     "to add them."
                  << std::endl;
        std::cerr << std::endl;
        std::cerr << "Usage:  " << "polygraph start [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
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

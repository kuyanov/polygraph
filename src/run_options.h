#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class RunOptions {
public:
    po::options_description desc{"Options"};
    std::string workflow_file;

    void HelpMessage() {
        std::cerr << "Command-line interface for running a workflow (represented as a .json file)"
                  << std::endl;
        std::cerr << std::endl;
        std::cerr << "Usage:  " << "polygraph run FILENAME [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        desc.add_options()("workflow", po::value<std::string>(&workflow_file)->required(),
                           "workflow filename");
        po::positional_options_description pos;
        pos.add("workflow", -1);
        po::variables_map vm;
        try {
            po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
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

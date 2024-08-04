#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "environment.h"

namespace po = boost::program_options;

class RunOptions {
public:
    po::options_description desc{"Options"};
    std::string workflow_file;
    std::string host;
    int port;

    void HelpMessage() {
        std::cerr << "Usage:  " << "polygraph run FILENAME [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << desc << std::endl;
        exit(EXIT_FAILURE);
    }

    void Init(int argc, char **argv) {
        desc.add_options()("help", "print help message");
        desc.add_options()("workflow", po::value<std::string>(&workflow_file)->required(),
                           "workflow filename");
        desc.add_options()("host", po::value<std::string>(&host)->default_value("127.0.0.1"),
                           "polygraph host");
        desc.add_options()("port", po::value<int>(&port)->default_value(3000), "polygraph port");
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

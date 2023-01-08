#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class Options {
public:
    std::string host;
    int port;

    static Options &Get() {
        static Options options;
        return options;
    }

    void Init(int argc, char **argv) {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "print help message");
        desc.add_options()("host", po::value<std::string>()->default_value("0.0.0.0"),
                           "host to listen on");
        desc.add_options()("port", po::value<int>()->default_value(3000), "port to listen on");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            exit(1);
        }
        host = vm["host"].as<std::string>();
        port = vm["port"].as<int>();
    }

private:
    Options() = default;
};

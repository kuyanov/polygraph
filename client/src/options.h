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
    std::string workflow;

    static Options &Get() {
        static Options options;
        return options;
    }

    void Init(int argc, char **argv) {
        po::options_description desc("Allowed options");
        desc.add_options()("help", "print help message");
        desc.add_options()("host", po::value<std::string>()->default_value("127.0.0.1"),
                           "scheduler host");
        desc.add_options()("port", po::value<int>()->default_value(3000), "scheduler port");
        desc.add_options()("workflow", po::value<std::string>(), "path to workflow file");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            exit(1);
        }
        if (!vm.count("workflow")) {
            std::cerr << "Missing required option 'workflow'" << std::endl;
            exit(1);
        }
        host = vm["host"].as<std::string>();
        port = vm["port"].as<int>();
        workflow = vm["workflow"].as<std::string>();
    }

private:
    Options() = default;
};

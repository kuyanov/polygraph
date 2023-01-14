#pragma once

#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class Options {
public:
    std::string host;
    int port;
    std::string partition;

    static Options &Get() {
        static Options options;
        return options;
    }

    void Init(int argc, char **argv) {
        po::options_description desc;
        desc.add_options()("host", po::value<std::string>()->default_value("127.0.0.1"));
        desc.add_options()("port", po::value<int>()->default_value(3000));
        desc.add_options()("partition", po::value<std::string>()->default_value("all"));
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        host = vm["host"].as<std::string>();
        port = vm["port"].as<int>();
        partition = vm["partition"].as<std::string>();
    }

private:
    Options() = default;
};

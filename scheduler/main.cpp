#include <iostream>
#include <unistd.h>
#include <boost/program_options.hpp>

#include "config.h"
#include "logger.h"
#include "run.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {
    po::options_description desc("Allowed options");
    desc.add_options()("help", "print help message");
    desc.add_options()("config", po::value<std::string>()->default_value(DEFAULT_CONFIG_PATH),
                       "path to config file");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cerr << desc << std::endl;
        return 1;
    }
    if (geteuid() != 0) {
        std::cerr << "pscheduler must be run as root, exiting." << std::endl;
        return 1;
    }
    Logger::Get().SetName("scheduler");
    Config::Get().Load(vm["config"].as<std::string>());
    Run();
}

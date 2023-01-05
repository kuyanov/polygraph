#include <csignal>
#include <iostream>
#include <memory>
#include <boost/program_options.hpp>

#include "client.h"
#include "json.h"

namespace po = boost::program_options;

std::unique_ptr<Client> client;

int main(int argc, char **argv) {
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
        return 1;
    }
    if (!vm.count("workflow")) {
        std::cerr << "Missing required option 'workflow'" << std::endl;
        return 1;
    }
    std::string workflow_path = vm["workflow"].as<std::string>();
    std::string scheduler_host = vm["host"].as<std::string>();
    int scheduler_port = vm["port"].as<int>();
    client = std::make_unique<Client>(ReadJSON(workflow_path), scheduler_host, scheduler_port);
    signal(SIGINT, [](int) { client->Stop(); });
    client->Run();
}

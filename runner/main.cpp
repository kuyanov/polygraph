#include <chrono>
#include <csignal>
#include <iostream>
#include <sys/prctl.h>
#include <thread>
#include <unistd.h>
#include <boost/program_options.hpp>

#include "config.h"
#include "logger.h"
#include "run.h"

namespace po = boost::program_options;

void StartLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "start", NULL);
}

void StopLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "stop", NULL);
}

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
    signal(SIGTERM, [](int) { StopLibsbox(); });
    if (fork() == 0) {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        StartLibsbox();
        perror("Failed to start libsbox");
    } else {
        Logger::Get().SetName("runner");
        Config::Get().Load(vm["config"].as<std::string>());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        Run();
    }
}

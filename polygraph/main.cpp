#include <cstring>
#include <functional>
#include <iostream>

#include "run.h"
#include "runner_start.h"
#include "runner_stop.h"
#include "start.h"
#include "stop.h"

template <class Options>
void InvokeWithOptions(int argc, char **argv, const std::function<void(Options)> &func) {
    Options options;
    options.Init(argc, argv);
    func(options);
}

void ParseRunnerAction(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        std::cerr << "Usage:  "
                  << "polygraph runner ACTION [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Actions:" << std::endl;
        std::cerr << "  start   "
                  << "Start runners" << std::endl;
        std::cerr << "  stop    "
                  << "Stop runners" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run 'polygraph runner ACTION --help' for more information about the action."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "start") == 0) {
        InvokeWithOptions<RunnerStartOptions>(argc - 1, argv + 1, RunnerStart);
    } else if (strcmp(argv[1], "stop") == 0) {
        InvokeWithOptions<RunnerStopOptions>(argc - 1, argv + 1, RunnerStop);
    } else {
        std::cerr << "Unknown action '" << argv[1] << "'." << std::endl;
        std::cerr << "See 'polygraph runner --help'." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ParseCommand(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        std::cerr << "Usage:  "
                  << "polygraph COMMAND [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Commands:" << std::endl;
        std::cerr << "  attach   "
                  << "Attach this machine to an already running polygraph service" << std::endl;
        std::cerr << "  detach   "
                  << "Stop all runners and detach from polygraph service" << std::endl;
        std::cerr << "  run      "
                  << "Run a workflow" << std::endl;
        std::cerr << "  runner   "
                  << "Manage runners" << std::endl;
        std::cerr << "  start    "
                  << "Start polygraph service on this machine" << std::endl;
        std::cerr << "  stop     "
                  << "Stop polygraph service and all runners on this machine" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run 'polygraph COMMAND --help' for more information about the command."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "run") == 0) {
        InvokeWithOptions<RunOptions>(argc - 1, argv + 1, Run);
    } else if (strcmp(argv[1], "runner") == 0) {
        ParseRunnerAction(argc - 1, argv + 1);
    } else if (strcmp(argv[1], "start") == 0) {
        InvokeWithOptions<StartOptions>(argc - 1, argv + 1, Start);
    } else if (strcmp(argv[1], "stop") == 0) {
        InvokeWithOptions<StopOptions>(argc - 1, argv + 1, Stop);
    } else {
        std::cerr << "Unknown command '" << argv[1] << "'." << std::endl;
        std::cerr << "See 'polygraph --help'." << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    ParseCommand(argc, argv);
}

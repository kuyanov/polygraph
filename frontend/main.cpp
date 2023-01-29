#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <unistd.h>

#include "config.h"
#include "options.h"

namespace fs = std::filesystem;

void RequireRoot() {
    if (geteuid() != 0) {
        std::cerr << "Error: this command requires root privileges." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Daemonize() {
    if (!fs::exists(GetLogDir())) {
        fs::create_directories(GetLogDir());
    }
    fs::path log_path = fs::path(GetLogDir()) / "common.log";
    FILE *fs_null = fopen("/dev/null", "r+");
    FILE *fs_log = fopen(log_path.c_str(), "a");
    if (!fs_null) {
        perror("Failed to open /dev/null");
        exit(EXIT_FAILURE);
    }
    if (!fs_log) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }
    dup2(fileno(fs_null), STDIN_FILENO);
    dup2(fileno(fs_null), STDOUT_FILENO);
    dup2(fileno(fs_log), STDERR_FILENO);
    fclose(fs_null);
    fclose(fs_log);
    if (daemon(1, 1)) {
        perror("Failed to daemonize");
        exit(EXIT_FAILURE);
    }
}

void ConfigGetCmd(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage:  " << argv[0] << " config get" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command displays config values used by polygraph." << std::endl;
        std::cerr << "To change them, run '" << argv[0] << " config set [--FIELD VALUE ...]'."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cerr << "host : " << Config::Get().scheduler_host << std::endl;
    std::cerr << "port : " << Config::Get().scheduler_port << std::endl;
}

void ConfigSetCmd(int argc, char **argv) {
    if (argc <= 3 || argc % 2 != 1) {
        std::cerr << "Usage:  " << argv[0] << " config set [--FIELD VALUE ...]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command changes config values used by polygraph." << std::endl;
        std::cerr << "For example, '" << argv[0]
                  << " config set --port 3001' will set the listen port to 3001." << std::endl;
        std::cerr << "To display current config values, run '" << argv[0] << " config get'."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    ConfigOptions::Get().Init(argc - 2, argv + 2);
    RequireRoot();
    ConfigOptions::Get().SetConfig();
    Config::Get().Dump();
}

void ConfigCmd(int argc, char **argv) {
    if (argc < 3 || strcmp(argv[2], "--help") == 0) {
        std::cerr << "Usage:  " << argv[0] << " config ACTION [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Actions:" << std::endl;
        std::cerr << "  get      "
                  << "Display config values" << std::endl;
        std::cerr << "  set      "
                  << "Update config values" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run '" << argv[0]
                  << " config ACTION --help' for more information about the action." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[2], "get") == 0) {
        ConfigGetCmd(argc, argv);
    } else if (strcmp(argv[2], "set") == 0) {
        ConfigSetCmd(argc, argv);
    } else {
        std::cerr << "Unknown action '" << argv[2] << "'." << std::endl;
        std::cerr << "See '" << argv[0] << " config --help'." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void RunCmd(int argc, char **argv) {
    if (argc < 3 || strcmp(argv[2], "--help") == 0) {
        std::cerr << "Usage:  " << argv[0] << " run WORKFLOW_PATH" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command runs a workflow given by its path WORKFLOW_PATH." << std::endl;
        exit(EXIT_FAILURE);
    }
    fs::path client_path = fs::path(GetExecDir()) / "pclient";
    execl(client_path.c_str(), "pclient", argv[2], nullptr);
    perror("Failed to run");
    exit(EXIT_FAILURE);
}

void RunnerAddCmd(int argc, char **argv) {
    // TODO
}

void RunnerListCmd(int argc, char **argv) {
    // TODO
}

void RunnerRemoveCmd(int argc, char **argv) {
    // TODO
}

void RunnerCmd(int argc, char **argv) {
    if (argc < 3 || strcmp(argv[2], "--help") == 0) {
        std::cerr << "Usage:  " << argv[0] << " runner ACTION [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Actions:" << std::endl;
        std::cerr << "  add      "
                  << "Connect a new runner" << std::endl;
        std::cerr << "  list     "
                  << "Show all runners" << std::endl;
        std::cerr << "  remove   "
                  << "Disconnect an existing runner" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run '" << argv[0]
                  << " runner ACTION --help' for more information about the action." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[2], "add") == 0) {
        RunnerAddCmd(argc, argv);
    } else if (strcmp(argv[2], "list") == 0) {
        RunnerListCmd(argc, argv);
    } else if (strcmp(argv[2], "remove") == 0) {
        RunnerRemoveCmd(argc, argv);
    } else {
        std::cerr << "Unknown action '" << argv[2] << "'." << std::endl;
        std::cerr << "See '" << argv[0] << " runner --help'." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void StartCmd(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage:  " << argv[0] << " start" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command starts polygraph service with no runners." << std::endl;
        std::cerr << "To start runners, you should execute '" << argv[0] << " runner add'."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    RequireRoot();
    if (Config::Get().scheduler_host == "127.0.0.1") {
        Daemonize();
        fs::path scheduler_path = fs::path(GetExecDir()) / "pscheduler";
        execl(scheduler_path.c_str(), "pscheduler", nullptr);
        perror("Failed to start scheduler");
        exit(EXIT_FAILURE);
    }
}

void StopCmd(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage:  " << argv[0] << " stop" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command stops polygraph service." << std::endl;
        exit(EXIT_FAILURE);
    }
    RequireRoot();
    if (system("pkill prunner")) {
        std::cerr << "Not killed prunner" << std::endl;
    }
    if (system("pkill pscheduler")) {
        std::cerr << "Not killed pscheduler" << std::endl;
    }
}

int main(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        std::cerr << "Usage:  " << argv[0] << " COMMAND [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Commands:" << std::endl;
        std::cerr << "  config   "
                  << "Manage config" << std::endl;
        std::cerr << "  run      "
                  << "Run workflow" << std::endl;
        std::cerr << "  runner   "
                  << "Manage runners" << std::endl;
        std::cerr << "  start    "
                  << "Start polygraph service" << std::endl;
        std::cerr << "  stop     "
                  << "Stop polygraph service" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run '" << argv[0]
                  << " COMMAND --help' for more information about the command." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "config") == 0) {
        ConfigCmd(argc, argv);
    } else if (strcmp(argv[1], "run") == 0) {
        RunCmd(argc, argv);
    } else if (strcmp(argv[1], "runner") == 0) {
        RunnerCmd(argc, argv);
    } else if (strcmp(argv[1], "start") == 0) {
        StartCmd(argc, argv);
    } else if (strcmp(argv[1], "stop") == 0) {
        StopCmd(argc, argv);
    } else {
        std::cerr << "Unknown command '" << argv[1] << "'." << std::endl;
        std::cerr << "See '" << argv[0] << " --help'." << std::endl;
        exit(EXIT_FAILURE);
    }
}

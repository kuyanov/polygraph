#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <unistd.h>

#include "client.h"
#include "config.h"
#include "options.h"

namespace fs = std::filesystem;

bool IsMasterNode() {
    return Config::Get().scheduler_host == "127.0.0.1";
}

bool IsSchedulerUp() {
    return fs::exists(fs::path(GetRunDir()) / "pscheduler.pid");
}

void RequireRoot() {
    if (geteuid() != 0) {
        std::cerr << "Error: this command requires root privileges." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void RequireUp() {
    if (IsMasterNode() && !IsSchedulerUp()) {
        std::cerr << "Error: polygraph service is not running." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void RequireDown() {
    if (IsMasterNode() && IsSchedulerUp()) {
        std::cerr << "Error: polygraph service is already running." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void CreateDirs() {
    fs::create_directories(fs::path(GetLogDir()));
    fs::create_directories(fs::path(GetRunDir()));
    fs::create_directories(fs::path(GetVarDir()));
}

void Daemonize() {
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
    std::cerr << "runner-reconnect-interval-ms : " << Config::Get().runner_reconnect_interval_ms
              << std::endl;
    std::cerr << "scheduler-host               : " << Config::Get().scheduler_host << std::endl;
    std::cerr << "scheduler-port               : " << Config::Get().scheduler_port << std::endl;
    std::cerr << "scheduler-max-payload-length : " << Config::Get().scheduler_max_payload_length
              << std::endl;
    std::cerr << "scheduler-idle-timeout       : " << Config::Get().scheduler_idle_timeout
              << std::endl;
}

void ConfigSetCmd(int argc, char **argv) {
    if (argc < 4 || strcmp(argv[3], "--help") == 0) {
        std::cerr << "Usage:  " << argv[0] << " config set [--FIELD VALUE ...]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command changes config values used by polygraph." << std::endl;
        std::cerr << "For example, '" << argv[0]
                  << " config set --scheduler-port 3001' will set the scheduler port to 3001."
                  << std::endl;
        std::cerr << "To display current config values, run '" << argv[0] << " config get'."
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    ConfigOptions::Get().Init(argc - 2, argv + 2);
    RequireRoot();
    RequireDown();
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

std::function<void()> interrupt_handler;

void RunCmd(int argc, char **argv) {
    if (argc < 3 || strcmp(argv[2], "--help") == 0) {
        std::cerr << "Usage:  " << argv[0] << " run WORKFLOW_PATH" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command runs the workflow located at WORKFLOW_PATH" << std::endl;
        exit(EXIT_FAILURE);
    }
    RequireUp();
    Client client(argv[2]);
    interrupt_handler = [&client] { client.Stop(); };
    signal(SIGINT, [](int) { interrupt_handler(); });
    client.Run();
}

void RunnerAttachCmd(int argc, char **argv) {
    if (argc < 3 || (argc >= 4 && strcmp(argv[3], "--help") == 0)) {
        std::cerr << "Usage:  " << argv[0] << " runner attach [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "This command attaches new runners to polygraph." << std::endl;
        exit(EXIT_FAILURE);
    }
    RequireRoot();
    RequireUp();
    Daemonize();
    fs::path pid_path = fs::path(GetRunDir()) / "prunner.pid";
    std::ofstream(pid_path.string()) << getpid() << std::endl;
    fs::path exec_path = fs::path(GetExecDir()) / "prunner";
    execl(exec_path.c_str(), "prunner", "all", nullptr);
    fs::remove(pid_path);
    perror("Failed to start runner");
    exit(EXIT_FAILURE);
}

void RunnerListCmd(int argc, char **argv) {
    // TODO
}

void RunnerDetachCmd(int argc, char **argv) {
    // TODO
}

void RunnerCmd(int argc, char **argv) {
    if (argc < 3 || strcmp(argv[2], "--help") == 0) {
        std::cerr << "Usage:  " << argv[0] << " runner ACTION [OPTIONS]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Actions:" << std::endl;
        std::cerr << "  attach   "
                  << "Attach new runners" << std::endl;
        std::cerr << "  list     "
                  << "Show all runners" << std::endl;
        std::cerr << "  detach   "
                  << "Detach existing runners" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Run '" << argv[0]
                  << " runner ACTION --help' for more information about the action." << std::endl;
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[2], "attach") == 0) {
        RunnerAttachCmd(argc, argv);
    } else if (strcmp(argv[2], "list") == 0) {
        RunnerListCmd(argc, argv);
    } else if (strcmp(argv[2], "detach") == 0) {
        RunnerDetachCmd(argc, argv);
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
        std::cerr << "To start runners, execute '" << argv[0] << " runner attach'." << std::endl;
        exit(EXIT_FAILURE);
    }
    RequireRoot();
    RequireDown();
    CreateDirs();
    if (IsMasterNode()) {
        Daemonize();
        fs::path pid_path = fs::path(GetRunDir()) / "pscheduler.pid";
        std::ofstream(pid_path.string()) << getpid() << std::endl;
        fs::path exec_path = fs::path(GetExecDir()) / "pscheduler";
        execl(exec_path.c_str(), "pscheduler", nullptr);
        fs::remove(pid_path);
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
    RequireUp();
    for (const auto &entry : fs::directory_iterator(fs::path(GetRunDir()))) {
        pid_t pid;
        std::ifstream(entry.path().string()) >> pid;
        if (kill(pid, SIGTERM)) {
            perror("Failed to kill");
            exit(EXIT_FAILURE);
        }
        fs::remove(entry.path());
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

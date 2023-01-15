#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sys/mount.h>
#include <unistd.h>

#include "constants.h"
#include "options.h"

namespace fs = std::filesystem;

void RequireRoot() {
    if (geteuid() != 0) {
        std::cerr << "polygraph must be run as root, exiting." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Daemonize() {
    fs::path log_path = fs::path(paths::kLogDir) / "common.log";
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

int main(int argc, char **argv) {
    GlobalOptions::Get().Init(argc, argv);
    if (GlobalOptions::Get().command == "run") {
        RunOptions::Get().Init(GlobalOptions::Get().command_options);
        fs::path client_path = fs::path(paths::kExecDir) / "pclient";
        execl(client_path.c_str(), "pclient", "--host", RunOptions::Get().host.c_str(), "--port",
              std::to_string(RunOptions::Get().port).c_str(), "--workflow",
              RunOptions::Get().workflow.c_str(), nullptr);
    } else if (GlobalOptions::Get().command == "runner") {
        RunnerOptions::Get().Init(GlobalOptions::Get().command_options);
        if (RunnerOptions::Get().action == "add") {
            RequireRoot();
        } else if (RunnerOptions::Get().action == "list") {

        } else if (RunnerOptions::Get().action == "remove") {
            RequireRoot();
        } else {
            std::cerr << "Unknown action '" << RunnerOptions::Get().action << "'." << std::endl;
            std::cerr << "See 'polygraph runner --help'." << std::endl;
        }
    } else if (GlobalOptions::Get().command == "start") {
        StartOptions::Get().Init(GlobalOptions::Get().command_options);
        RequireRoot();
        fs::path source_user_dir = StartOptions::Get().user_dir;
        fs::path internal_user_dir = fs::path(paths::kVarDir) / "user";
        if (!fs::exists(source_user_dir)) {
            std::cerr << "Directory " << source_user_dir << " not exists, creating it."
                      << std::endl;
            fs::create_directories(source_user_dir);
            fs::permissions(source_user_dir, fs::perms::all);
        }
        fs::create_directory(internal_user_dir);
        if (mount(source_user_dir.c_str(), internal_user_dir.c_str(), "", MS_BIND, "")) {
            perror("Failed to mount user directory");
            exit(EXIT_FAILURE);
        }
        if (StartOptions::Get().host == "127.0.0.1") {
            Daemonize();
            fs::path scheduler_path = fs::path(paths::kExecDir) / "pscheduler";
            execl(scheduler_path.c_str(), "pscheduler", "--port",
                  std::to_string(StartOptions::Get().port).c_str(), nullptr);
            perror("Failed to start scheduler");
            exit(EXIT_FAILURE);
        }
    } else if (GlobalOptions::Get().command == "stop") {
        StopOptions::Get().Init(GlobalOptions::Get().command_options);
        RequireRoot();
        if (system("pkill prunner")) {
            std::cerr << "Not killed prunner" << std::endl;
        }
        if (system("pkill pscheduler")) {
            std::cerr << "Not killed pscheduler" << std::endl;
        }
        fs::path user_dir = fs::path(paths::kVarDir) / "user";
        if (umount(user_dir.c_str())) {
            perror("Failed to unmount user directory");
            exit(EXIT_FAILURE);
        }
    } else {
        std::cerr << "Unknown command '" << GlobalOptions::Get().command << "'." << std::endl;
        std::cerr << "See 'polygraph --help'." << std::endl;
    }
}

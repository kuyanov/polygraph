#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>

#include "helpers.h"
#include "logger.h"
#include "runner.h"
#include "runner_start.h"

namespace fs = std::filesystem;

void RunnerStart(const RunnerStartOptions &options) {
    RequireRoot();
    RequireUp();
    CreateDirs();
    int runner_id = 0;
    for (int iter = 0; iter < options.num; ++iter) {
        fs::path pid_path;
        while (fs::exists(pid_path = fs::path(RUN_DIR) /
                                     ("runner_" + std::to_string(runner_id) + ".pid"))) {
            ++runner_id;
        }
        std::ofstream pid_file(pid_path.string());
        int child_pid = fork();
        if (child_pid < 0) {
            perror("Failed to fork");
            exit(EXIT_FAILURE);
        }
        if (child_pid == 0) {
            Daemonize();
            pid_file << getpid() << std::endl;
            auto runner = Runner(std::to_string(runner_id), options.partition);
            runner.Run();
        }
    }
}

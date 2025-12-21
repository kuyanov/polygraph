#include <filesystem>
#include <fstream>
#include <unistd.h>

#include "helpers.h"
#include "runner_start.h"
#include "runner_start_options.h"
#include "scheduler_app.h"
#include "start.h"

namespace fs = std::filesystem;

void Start(const StartOptions &options) {
    RequireRoot();
    RequireDown();
    fs::path pid_path = fs::path(RUN_DIR) / "scheduler.pid";
    std::ofstream pid_file(pid_path.string());
    RunnerStartOptions runner_options;
    runner_options.num = options.num_runners;
    runner_options.partition = options.partition;
    RunnerStart(runner_options);
    Daemonize();
    pid_file << getpid() << std::endl;
    SchedulerApp::Get().Run();
}

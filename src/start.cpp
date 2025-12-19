#include <filesystem>
#include <fstream>
#include <unistd.h>

#include "helpers.h"
#include "scheduler_app.h"
#include "start.h"

namespace fs = std::filesystem;

void Start(const StartOptions &options) {
    RequireRoot();
    RequireDown();
    Daemonize();
    fs::path pid_path = fs::path(RUN_DIR) / "scheduler.pid";
    std::ofstream pid_file(pid_path.string());
    pid_file << getpid() << std::endl;
    SchedulerApp::Get().Run();
}

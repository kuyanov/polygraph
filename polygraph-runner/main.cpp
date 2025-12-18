#include <csignal>
#include <cstdlib>
#include <string>

#include "environment.h"
#include "logger.h"
#include "run.h"

void InterruptHandler(int signum) {
    Log("Terminated with signal ", signum);
    exit(signum);
}

int main() {
    std::string id = GetEnvOr("RUNNER_ID", "0");
    std::string partition = GetEnvOr("RUNNER_PARTITION", "all");
    std::string name = std::string("runner ") + id;
    Logger::Get().SetName(name);
    signal(SIGINT, InterruptHandler);
    signal(SIGTERM, InterruptHandler);
    Run(id, partition);
}

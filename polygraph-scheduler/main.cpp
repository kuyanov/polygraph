#include <csignal>
#include <cstdlib>

#include "logger.h"
#include "run.h"

void InterruptHandler(int signum) {
    Log("Terminated with signal ", signum);
    exit(signum);
}

int main() {
    Logger::Get().SetName("scheduler");
    signal(SIGINT, InterruptHandler);
    signal(SIGTERM, InterruptHandler);
    Run();
}

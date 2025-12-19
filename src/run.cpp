#include <csignal>
#include <functional>

#include "client.h"
#include "helpers.h"
#include "run.h"

std::function<void(int)> interrupt_handler;

void Run(const RunOptions &options) {
    RequireUp();
    Client client(options.workflow_file);
    interrupt_handler = [&client](int signum) { client.Stop(); };
    signal(SIGINT, [](int signum) { interrupt_handler(signum); });
    client.Run();
}

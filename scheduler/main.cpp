#include <iostream>
#include <unistd.h>

#include "logger.h"
#include "options.h"
#include "run.h"

int main(int argc, char **argv) {
    if (geteuid() != 0) {
        std::cerr << "pscheduler must be run as root, exiting." << std::endl;
        return 1;
    }
    Options::Get().Init(argc, argv);
    Logger::Get().SetName("scheduler");
    Run();
}

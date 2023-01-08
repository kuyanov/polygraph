#include <chrono>
#include <csignal>
#include <iostream>
#include <sys/prctl.h>
#include <thread>
#include <unistd.h>

#include "logger.h"
#include "options.h"
#include "run.h"

void StartLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "start", NULL);
}

void StopLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "stop", NULL);
}

int main(int argc, char **argv) {
    if (geteuid() != 0) {
        std::cerr << "prunner must be run as root, exiting." << std::endl;
        return 1;
    }
    Options::Get().Init(argc, argv);
    signal(SIGTERM, [](int) { StopLibsbox(); });
    if (fork() == 0) {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        StartLibsbox();
        perror("Failed to start libsbox");
    } else {
        Logger::Get().SetName("runner");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // best synchronization ever
        Run();
    }
}

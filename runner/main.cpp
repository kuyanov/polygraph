#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/prctl.h>
#include <thread>
#include <unistd.h>

#include "logger.h"
#include "run.h"

void StartLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "start", NULL);
}

void StopLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "stop", NULL);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage:  prunner PARTITION" << std::endl;
        exit(EXIT_FAILURE);
    }
    signal(SIGTERM, [](int) { StopLibsbox(); });
    if (fork() == 0) {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        StartLibsbox();
        perror("Failed to start libsbox");
        exit(EXIT_FAILURE);
    } else {
        Logger::Get().SetName("runner");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // best synchronization ever
        Run(argv[1]);
    }
}

#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "run.h"

void StartLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "start", NULL);
}

void StopLibsbox() {
    execl("/usr/bin/libsboxd", "libsboxd", "stop", NULL);
}

int main() {
    signal(SIGTERM, [](int) { StopLibsbox(); });
    if (fork() == 0) {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        StartLibsbox();
    } else {
        Run();
    }
}

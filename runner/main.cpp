#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "run.h"

int main() {
    if (fork() == 0) {
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        execl("/usr/bin/libsboxd", "libsboxd", "start", NULL);
    } else {
        Run();
    }
}

#include "logger.h"
#include "options.h"
#include "run.h"

int main(int argc, char **argv) {
    Options::Get().Init(argc, argv);
    Logger::Get().SetName("scheduler");
    Run();
}

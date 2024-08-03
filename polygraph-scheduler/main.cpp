#include "logger.h"
#include "run.h"

int main() {
    Logger::Get().SetName("scheduler");
    Run();
}

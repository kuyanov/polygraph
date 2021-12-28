#include "config.h"
#include "run.h"

int main() {
    Config config = ConfigFromFile("config.json");
    Run(config);
}

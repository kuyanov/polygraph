#include "config.h"
#include "run.h"

int main() {
    Config config(std::string(MASTERNODE_ROOT_DIR) + "/config.json");
    Run(config);
}

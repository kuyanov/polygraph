#include "config_set.h"
#include "helpers.h"

void ConfigSet(const ConfigSetOptions &options) {
    RequireRoot();
    RequireDown();
    auto config = Config::Get();
    config.Dump();
}

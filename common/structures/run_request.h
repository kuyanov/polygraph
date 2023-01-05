#pragma once

#include <string>
#include <vector>

#include "bind.h"
#include "constraints.h"

struct RunRequest {
    std::vector<Bind> binds;
    std::vector<std::string> argv, env;
    Constraints constraints;
};

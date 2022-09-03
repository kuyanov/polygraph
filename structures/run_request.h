#pragma once

#include <string>
#include <vector>

#include "task.h"

struct RunRequest {
    std::string container;
    std::vector<Task> tasks;
};

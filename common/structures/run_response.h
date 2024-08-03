#pragma once

#include <optional>
#include <string>

#include "run_status.h"

struct RunResponse {
    std::optional<std::string> error;
    std::optional<RunStatus> status;
};

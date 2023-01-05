#pragma once

#include <optional>
#include <string>

#include "status.h"

struct RunResponse {
    std::optional<std::string> error;
    std::optional<Status> status;
};

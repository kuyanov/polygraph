#pragma once

#include <optional>
#include <string>

#include "run_status.h"

struct BlockResponse {
    size_t block_id;
    std::string state;
    std::optional<std::string> error;
    std::optional<RunStatus> status;
};

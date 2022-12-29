#pragma once

#include <optional>
#include <string>

#include "status.h"

struct BlockResponse {
    size_t block_id;
    std::string state;
    std::optional<std::string> error;
    std::optional<Status> status;
};

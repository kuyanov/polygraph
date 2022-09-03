#pragma once

#include <string>
#include <vector>

#include "result.h"

struct RunResponse {
    bool has_error;
    std::string error;
    std::vector<Result> results;
};

struct BlockRunResponse {
    RunResponse run_response;
    size_t block_id;
};

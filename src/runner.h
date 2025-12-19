#pragma once

#include <string>

class Runner {
public:
    Runner(const std::string &id, const std::string &partition);

    void Run();

private:
    std::string id_, partition_;
};

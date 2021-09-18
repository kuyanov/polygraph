#pragma once

#include <algorithm>
#include <random>
#include <sstream>

namespace uuid {
static std::random_device rnd;
static std::mt19937 gen(rnd());
static std::uniform_int_distribution<> dis1(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

std::string Generate() {
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {
        ss << dis1(gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis1(gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << dis1(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis1(gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis1(gen);
    }
    return ss.str();
}
}  // namespace uuid

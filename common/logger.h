#pragma once

#include <iostream>
#include <string>
#include <utility>

class Logger {
public:
    static Logger &Get() {
        static Logger logger;
        return logger;
    }

    void SetName(const std::string &name) {
        name_ = name;
    }

    template <class... Args>
    void Print(std::ostream &out, Args... args) {
        out << "[" << name_ << "] ";
        ((out << args), ...);
        out << std::endl;
    }

private:
    std::string name_;
    Logger() = default;
};

template <class... Args>
void Log(Args... args) {
    Logger::Get().template Print(std::cerr, std::forward<Args>(args)...);
}

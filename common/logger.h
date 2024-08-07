#pragma once

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
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

    void Print(std::ostream &out, const std::string &text) {
        time_t now = time(nullptr);
        std::stringstream ss;
        ss << std::put_time(localtime(&now), "%F %T");
        ss << " [" << name_ << "] " << text << "\n";
        out << ss.str();
        out.flush();
    }

private:
    std::string name_;
    Logger() = default;
};

template <class... Args>
inline std::string JoinToString(Args... args) {
    std::stringstream ss;
    ((ss << args), ...);
    return ss.str();
}

template <class... Args>
inline void Log(Args... args) {
    Logger::Get().Print(std::clog, JoinToString(std::forward<Args>(args)...));
}

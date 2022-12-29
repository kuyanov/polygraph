#pragma once

#include <iostream>
#include <string>

enum Color { GREY, RED, GREEN, YELLOW };

inline std::string ColoredText(const std::string &text, Color color) {
    switch (color) {
        case GREY:
            return "\033[30m" + text + "\033[0m";
        case RED:
            return "\033[31m" + text + "\033[0m";
        case GREEN:
            return "\033[32m" + text + "\033[0m";
        case YELLOW:
            return "\033[33m" + text + "\033[0m";
        default:
            return text;
    }
}

class TerminalWindow {
public:
    static TerminalWindow &Get() {
        static TerminalWindow window;
        return window;
    }

    void Clear() {
        for (int i = 0; i < cnt_lines_; ++i) {
            std::cout << "\x1b[1A";
        }
        for (int i = 0; i < cnt_lines_; ++i) {
            for (int j = 0; j < width_; ++j) {
                std::cout << ' ';
            }
            std::cout << std::endl;
        }
        for (int i = 0; i < cnt_lines_; ++i) {
            std::cout << "\x1b[1A";
        }
        cnt_lines_ = 0;
    }

    void PrintLine(const std::string &line) {
        std::cout << line << std::endl;
        ++cnt_lines_;
    }

private:
    int width_ = 50;
    int cnt_lines_ = 0;
};

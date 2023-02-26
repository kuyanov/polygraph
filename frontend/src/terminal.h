#pragma once

#include <iostream>
#include <string>

enum Color { RED, GREEN, YELLOW };

inline std::string ColoredText(const std::string &text, Color color) {
    switch (color) {
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

inline size_t DisplayedWidth(const std::string &text) {
    size_t width = 0;
    bool color_char = false;
    for (char c : text) {
        if (c == '\033') {
            color_char = true;
        } else if (color_char && c == 'm') {
            color_char = false;
        } else if (!color_char) {
            ++width;
        }
    }
    return width;
}

inline std::string AlignCenter(const std::string &text, size_t width) {
    size_t text_width = DisplayedWidth(text);
    if (width < text_width) {
        return text;
    }
    size_t left_padding = (width - text_width) / 2;
    size_t right_padding = (width - text_width + 1) / 2;
    return std::string(left_padding, ' ') + text + std::string(right_padding, ' ');
}

inline std::string AlignLeft(const std::string &text, size_t width) {
    size_t text_width = DisplayedWidth(text);
    if (width < text_width) {
        return text;
    }
    size_t right_padding = width - text_width;
    return text + std::string(right_padding, ' ');
}

class TerminalWindow {
public:
    static TerminalWindow &Get() {
        static TerminalWindow window;
        return window;
    }

    void Clear() {
        for (int i = 0; i < cnt_lines_; ++i) {
            std::cerr << "\x1b[1A";
        }
        cnt_lines_ = 0;
    }

    void PrintLine(const std::string &line) {
        std::cerr << line << std::endl;
        ++cnt_lines_;
    }

private:
    int cnt_lines_ = 0;
};

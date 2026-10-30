#ifndef INPUT_H
#define INPUT_H

#include <string>

namespace Input {
    std::string text(const std::string& prompt);
    int integer(const std::string& prompt, int minimum = 0);
    double money(const std::string& prompt);
    bool yesNo(const std::string& prompt);
}

#endif

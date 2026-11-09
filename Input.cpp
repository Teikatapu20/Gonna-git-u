#include "Input.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

// Largest unit price accepted; stops typos like 99999999999 from skewing totals.
const double MAX_PRICE = 1000000000.0;

std::string trim(const std::string& text) {
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

std::string toLower(std::string text) {
    for (char& c : text) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

// Shows the prompt and reads one line. Returns false when input has ended
// (Ctrl+Z / Ctrl+D or piped input ran out), so loops never spin forever.
bool readLine(const std::string& prompt, std::string& line) {
    std::cout << prompt;
    if (!std::getline(std::cin, line)) {
        std::cout << '\n';
        return false;
    }
    return true;
}

// Whole numbers only: optional +/- sign, then digits. No decimals or letters.
bool parseInt(const std::string& text, int& result) {
    std::size_t i = 0;
    bool negative = false;
    if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
        negative = (text[i] == '-');
        ++i;
    }
    if (i == text.size()) return false;

    long long value = 0;
    for (; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
        value = value * 10 + (text[i] - '0');
        if (value > std::numeric_limits<int>::max()) return false;
    }
    result = static_cast<int>(negative ? -value : value);
    return true;
}

// Plain decimal numbers only: digits with at most one dot (e.g. 250, 99.50, .5).
bool parseMoney(const std::string& text, double& result) {
    bool seenDigit = false;
    bool seenDot = false;
    for (char c : text) {
        if (std::isdigit(static_cast<unsigned char>(c))) seenDigit = true;
        else if (c == '.' && !seenDot) seenDot = true;
        else return false;
    }
    if (!seenDigit) return false;

    const double value = std::strtod(text.c_str(), nullptr);
    if (!std::isfinite(value) || value > MAX_PRICE) return false;
    result = value;
    return true;
}

}  // namespace

namespace Input {

std::string text(const std::string& prompt) {
    std::string line;
    while (readLine(prompt, line)) {
        line = trim(line);
        if (!line.empty()) return line;
        std::cout << "This field cannot be blank. Please try again.\n";
    }
    return "";  // input ended
}

int integer(const std::string& prompt, int minimum) {
    std::string line;
    while (readLine(prompt, line)) {
        int value = 0;
        if (!parseInt(trim(line), value)) {
            std::cout << "Please enter a whole number (for example 5).\n";
        } else if (value < minimum) {
            std::cout << "Please enter a number that is " << minimum << " or higher.\n";
        } else {
            return value;
        }
    }
    return minimum;  // input ended; for the menu this means "Save and exit"
}

double money(const std::string& prompt) {
    std::string line;
    while (readLine(prompt, line)) {
        double value = 0.0;
        if (parseMoney(trim(line), value)) return value;
        std::cout << "Please enter a valid price, zero or higher, using digits only (for example 499.99).\n";
    }
    return 0.0;  // input ended
}

bool yesNo(const std::string& prompt) {
    std::string line;
    while (readLine(prompt + " (y/n): ", line)) {
        const std::string answer = toLower(trim(line));
        if (answer == "y" || answer == "yes") return true;
        if (answer == "n" || answer == "no") return false;
        std::cout << "Please answer y or n.\n";
    }
    return false;  // input ended: do nothing destructive
}

}  // namespace Input

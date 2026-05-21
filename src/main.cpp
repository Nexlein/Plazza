/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Entry point
*/

#include <cstddef>
#include <iostream>
#include <string>

#include "plazza/reception/Reception.hpp"

namespace {

    void printUsage(const char* argv0)
    {
        std::cerr << "usage: " << argv0 << " <multiplier> <cooks_per_kitchen> <restock_ms>\n";
    }

    bool parsePositiveFloat(const char* arg, float& out)
    {
        try {
            std::string str(arg);
            std::size_t pos = 0;
            const float value = std::stof(str, &pos);
            if (pos != str.size() || value <= 0.f) return false;
            out = value;
            return true;
        } catch (...) {
            return false;
        }
    }

    bool parsePositiveSize(const char* arg, std::size_t& out)
    {
        try {
            std::string str(arg);
            std::size_t pos = 0;
            const long long value = std::stoll(str, &pos);
            if (pos != str.size() || value <= 0) return false;
            out = static_cast<std::size_t>(value);
            return true;
        } catch (...) {
            return false;
        }
    }

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 4) {
        printUsage(argv[0]);
        return 84;
    }

    float multiplier = 0.f;
    std::size_t cooks = 0;
    std::size_t restockMs = 0;

    if (!parsePositiveFloat(argv[1], multiplier)) {
        std::cerr << "error: multiplier must be a positive number\n";
        return 84;
    }
    if (!parsePositiveSize(argv[2], cooks)) {
        std::cerr << "error: cooks_per_kitchen must be a positive integer\n";
        return 84;
    }
    if (!parsePositiveSize(argv[3], restockMs)) {
        std::cerr << "error: restock_ms must be a positive integer\n";
        return 84;
    }

    plazza::reception::Reception reception(multiplier, cooks, restockMs);
    return reception.run();
}

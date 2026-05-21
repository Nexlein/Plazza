/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** PizzaSize
*/

#pragma once

#include <optional>
#include <string_view>

namespace plazza::pizza {

    enum class PizzaSize {
        S = 1,
        M = 2,
        L = 4,
        XL = 8,
        XXL = 16,
    };

    std::optional<PizzaSize> sizeFromString(std::string_view s);
    std::string_view sizeToString(PizzaSize s);

}  // namespace plazza::pizza

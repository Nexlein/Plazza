/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** PizzaType
*/

#pragma once

#include <optional>
#include <string_view>

namespace plazza::pizza {

    enum class PizzaType {
        Regina = 1,
        Margarita = 2,
        Americana = 4,
        Fantasia = 8,
    };

    std::optional<PizzaType> typeFromString(std::string_view s);
    std::string_view typeToString(PizzaType t);

}  // namespace plazza::pizza

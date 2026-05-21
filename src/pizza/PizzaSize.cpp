/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** PizzaSize
*/

#include "plazza/pizza/PizzaSize.hpp"

namespace plazza::pizza {

    std::optional<PizzaSize> sizeFromString(std::string_view s)
    {
        if (s == "S") return PizzaSize::S;
        if (s == "M") return PizzaSize::M;
        if (s == "L") return PizzaSize::L;
        if (s == "XL") return PizzaSize::XL;
        if (s == "XXL") return PizzaSize::XXL;
        return std::nullopt;
    }

    std::string_view sizeToString(PizzaSize s)
    {
        switch (s) {
            case PizzaSize::S:
                return "S";
            case PizzaSize::M:
                return "M";
            case PizzaSize::L:
                return "L";
            case PizzaSize::XL:
                return "XL";
            case PizzaSize::XXL:
                return "XXL";
        }
        return "?";
    }

}  // namespace plazza::pizza

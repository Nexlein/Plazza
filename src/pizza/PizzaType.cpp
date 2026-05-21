/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** PizzaType
*/

#include "plazza/pizza/PizzaType.hpp"

#include <cctype>
#include <string>

namespace plazza::pizza {

    namespace {

        std::string toLower(std::string_view s)
        {
            std::string out;
            out.reserve(s.size());
            for (char c : s)
                out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            return out;
        }

    }  // namespace

    std::optional<PizzaType> typeFromString(std::string_view s)
    {
        if (s.empty()) return std::nullopt;
        for (char c : s) {
            if (!std::isalpha(static_cast<unsigned char>(c))) return std::nullopt;
        }
        const std::string lower = toLower(s);
        if (lower == "regina") return PizzaType::Regina;
        if (lower == "margarita") return PizzaType::Margarita;
        if (lower == "americana") return PizzaType::Americana;
        if (lower == "fantasia") return PizzaType::Fantasia;
        return std::nullopt;
    }

    std::string_view typeToString(PizzaType t)
    {
        switch (t) {
            case PizzaType::Regina:
                return "regina";
            case PizzaType::Margarita:
                return "margarita";
            case PizzaType::Americana:
                return "americana";
            case PizzaType::Fantasia:
                return "fantasia";
        }
        return "?";
    }

}  // namespace plazza::pizza

/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Pizza
*/

#include "plazza/pizza/Pizza.hpp"

#include <stdexcept>

std::string_view plazza::pizza::ingredientToString(Ingredient i)
{
    switch (i) {
        case Ingredient::Dough:
            return "dough";
        case Ingredient::Tomato:
            return "tomato";
        case Ingredient::Gruyere:
            return "gruyere";
        case Ingredient::Ham:
            return "ham";
        case Ingredient::Mushrooms:
            return "mushrooms";
        case Ingredient::Steak:
            return "steak";
        case Ingredient::Eggplant:
            return "eggplant";
        case Ingredient::GoatCheese:
            return "goat cheese";
        case Ingredient::ChiefLove:
            return "chief love";
    }
    return "?";
}

const plazza::pizza::Recipe& plazza::pizza::recipeFor(PizzaType t)
{
    static const Recipe rMargarita{
        {Ingredient::Dough, Ingredient::Tomato, Ingredient::Gruyere},
        3,
        1,
    };
    static const Recipe rRegina{
        {Ingredient::Dough, Ingredient::Tomato, Ingredient::Gruyere, Ingredient::Ham,
         Ingredient::Mushrooms},
        5,
        2,
    };
    static const Recipe rAmericana{
        {Ingredient::Dough, Ingredient::Tomato, Ingredient::Gruyere, Ingredient::Steak},
        4,
        2,
    };
    static const Recipe rFantasia{
        {Ingredient::Dough, Ingredient::Tomato, Ingredient::Eggplant, Ingredient::GoatCheese,
         Ingredient::ChiefLove},
        5,
        4,
    };

    switch (t) {
        case PizzaType::Margarita:
            return rMargarita;
        case PizzaType::Regina:
            return rRegina;
        case PizzaType::Americana:
            return rAmericana;
        case PizzaType::Fantasia:
            return rFantasia;
    }
    throw std::invalid_argument("unknown PizzaType");
}

plazza::pizza::Pizza plazza::pizza::makePizza(PizzaType type, PizzaSize size)
{
    return {type, size, recipeFor(type)};
}

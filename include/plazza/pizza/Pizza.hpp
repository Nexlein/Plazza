/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Pizza
*/

#pragma once

#include <array>
#include <cstddef>
#include <string_view>
#include <type_traits>

#include "plazza/pizza/PizzaSize.hpp"
#include "plazza/pizza/PizzaType.hpp"

namespace plazza::pizza {

    enum class Ingredient {
        Dough,
        Tomato,
        Gruyere,
        Ham,
        Mushrooms,
        Steak,
        Eggplant,
        GoatCheese,
        ChiefLove,
    };

    std::string_view ingredientToString(Ingredient i);

    struct Recipe {
        static constexpr std::size_t MaxIngredients = 5;
        std::array<Ingredient, MaxIngredients> ingredients{};
        std::size_t ingredientCount{0};
        int baseBakeTime{0};
    };

    static_assert(std::is_trivially_copyable_v<Recipe>);

    struct Pizza {
        PizzaType type;
        PizzaSize size;
        Recipe recipe;
    };

    static_assert(std::is_trivially_copyable_v<Pizza>);

    const Recipe& recipeFor(PizzaType t);
    Pizza makePizza(PizzaType type, PizzaSize size);

}  // namespace plazza::pizza

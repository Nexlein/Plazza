/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_recipe
*/

#include <catch2/catch_test_macros.hpp>

#include "plazza/pizza/Pizza.hpp"

using namespace plazza::pizza;

TEST_CASE("Margarita recipe: 3 ingredients, 1s base bake", "[pizza][recipe]")
{
    const Recipe& r = recipeFor(PizzaType::Margarita);
    CHECK(r.baseBakeTime == 1);
    REQUIRE(r.ingredientCount == 3);
    CHECK(r.ingredients[0] == Ingredient::Dough);
    CHECK(r.ingredients[1] == Ingredient::Tomato);
    CHECK(r.ingredients[2] == Ingredient::Gruyere);
}

TEST_CASE("Regina recipe: 5 ingredients, 2s base bake", "[pizza][recipe]")
{
    const Recipe& r = recipeFor(PizzaType::Regina);
    CHECK(r.baseBakeTime == 2);
    REQUIRE(r.ingredientCount == 5);
    CHECK(r.ingredients[0] == Ingredient::Dough);
    CHECK(r.ingredients[1] == Ingredient::Tomato);
    CHECK(r.ingredients[2] == Ingredient::Gruyere);
    CHECK(r.ingredients[3] == Ingredient::Ham);
    CHECK(r.ingredients[4] == Ingredient::Mushrooms);
}

TEST_CASE("Americana recipe: 4 ingredients, 2s base bake", "[pizza][recipe]")
{
    const Recipe& r = recipeFor(PizzaType::Americana);
    CHECK(r.baseBakeTime == 2);
    REQUIRE(r.ingredientCount == 4);
    CHECK(r.ingredients[0] == Ingredient::Dough);
    CHECK(r.ingredients[1] == Ingredient::Tomato);
    CHECK(r.ingredients[2] == Ingredient::Gruyere);
    CHECK(r.ingredients[3] == Ingredient::Steak);
}

TEST_CASE("Fantasia recipe: 5 ingredients, 4s base bake", "[pizza][recipe]")
{
    const Recipe& r = recipeFor(PizzaType::Fantasia);
    CHECK(r.baseBakeTime == 4);
    REQUIRE(r.ingredientCount == 5);
    CHECK(r.ingredients[0] == Ingredient::Dough);
    CHECK(r.ingredients[1] == Ingredient::Tomato);
    CHECK(r.ingredients[2] == Ingredient::Eggplant);
    CHECK(r.ingredients[3] == Ingredient::GoatCheese);
    CHECK(r.ingredients[4] == Ingredient::ChiefLove);
}

TEST_CASE("makePizza bundles type, size, and correct recipe", "[pizza]")
{
    const Pizza p = makePizza(PizzaType::Americana, PizzaSize::L);
    CHECK(p.type == PizzaType::Americana);
    CHECK(p.size == PizzaSize::L);
    CHECK(p.recipe.baseBakeTime == 2);
    CHECK(p.recipe.ingredientCount == 4);
}

TEST_CASE("Pizza and Recipe are trivially copyable", "[pizza]")
{
    static_assert(std::is_trivially_copyable_v<Recipe>);
    static_assert(std::is_trivially_copyable_v<Pizza>);
}

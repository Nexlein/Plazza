/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_stock
*/

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>
#include <vector>

#include "plazza/kitchen/Stock.hpp"
#include "plazza/pizza/Pizza.hpp"

using namespace plazza::kitchen;
using namespace plazza::pizza;
using namespace std::chrono_literals;

TEST_CASE("Stock starts with 5 of every ingredient", "[stock]")
{
    Stock stock;
    for (auto i : {Ingredient::Dough, Ingredient::Tomato, Ingredient::Gruyere, Ingredient::Ham,
                   Ingredient::Mushrooms, Ingredient::Steak, Ingredient::Eggplant,
                   Ingredient::GoatCheese, Ingredient::ChiefLove}) {
        CHECK(stock.available(i) == Stock::kInitialPerIngredient);
    }
}

TEST_CASE("Stock::tryConsume decrements every ingredient in the recipe", "[stock]")
{
    Stock stock;
    const Recipe& regina = recipeFor(PizzaType::Regina);

    CHECK(stock.tryConsume(regina));
    CHECK(stock.available(Ingredient::Dough) == 4);
    CHECK(stock.available(Ingredient::Tomato) == 4);
    CHECK(stock.available(Ingredient::Gruyere) == 4);
    CHECK(stock.available(Ingredient::Ham) == 4);
    CHECK(stock.available(Ingredient::Mushrooms) == 4);
    // Ingredients not in the recipe are untouched.
    CHECK(stock.available(Ingredient::Steak) == 5);
    CHECK(stock.available(Ingredient::Eggplant) == 5);
}

TEST_CASE("Stock::tryConsume refuses partial consumption", "[stock]")
{
    Stock stock;
    const Recipe& regina = recipeFor(PizzaType::Regina);

    // Drain Ham — Regina cannot be consumed even though every other ingredient is present.
    for (int i = 0; i < 5; ++i) CHECK(stock.tryConsume(recipeFor(PizzaType::Americana)));
    CHECK(stock.available(Ingredient::Steak) == 0);

    // Refill everything except Ham to isolate the limiter — easiest: consume
    // until any one ingredient blocks. After 5 Americana: Dough=0, Tomato=0,
    // Gruyere=0, Steak=0. Any further regina must fail and leave counts unchanged.
    const std::size_t doughBefore = stock.available(Ingredient::Dough);
    const std::size_t hamBefore = stock.available(Ingredient::Ham);
    CHECK(!stock.tryConsume(regina));
    CHECK(stock.available(Ingredient::Dough) == doughBefore);
    CHECK(stock.available(Ingredient::Ham) == hamBefore);
}

TEST_CASE("Stock under concurrent tryConsume never oversells", "[stock][concurrency]")
{
    Stock stock;
    const Recipe& regina = recipeFor(PizzaType::Regina);

    std::atomic<int> successes{0};
    std::vector<std::thread> threads;
    threads.reserve(50);
    for (int t = 0; t < 50; ++t) {
        threads.emplace_back([&]() {
            while (stock.tryConsume(regina)) successes.fetch_add(1);
        });
    }
    for (auto& th : threads) th.join();

    // Regina consumes one of each of {Dough, Tomato, Gruyere, Ham, Mushrooms}.
    // With 5 initial of each, exactly 5 succeed.
    CHECK(successes.load() == 5);
    CHECK(stock.available(Ingredient::Dough) == 0);
    CHECK(stock.available(Ingredient::Ham) == 0);
    CHECK(stock.available(Ingredient::Mushrooms) == 0);
}

TEST_CASE("Stock::waitAndConsume wakes up after restockOnce", "[stock][blocking]")
{
    Stock stock;
    const Recipe& margarita = recipeFor(PizzaType::Margarita);

    // Drain Dough so margarita is blocked.
    for (int i = 0; i < 5; ++i) CHECK(stock.tryConsume(margarita));
    CHECK(stock.available(Ingredient::Dough) == 0);

    std::atomic<bool> stop{false};
    std::atomic<bool> consumed{false};
    std::thread waiter([&]() { consumed.store(stock.waitAndConsume(margarita, stop)); });

    // Without a restock, the waiter must stay parked.
    std::this_thread::sleep_for(150ms);
    CHECK_FALSE(consumed.load());

    stock.restockOnce();

    auto deadline = std::chrono::steady_clock::now() + 1s;
    while (!consumed.load() && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(10ms);
    CHECK(consumed.load());
    waiter.join();
}

TEST_CASE("Stock::waitAndConsume returns false when stop is signaled", "[stock][blocking]")
{
    Stock stock;
    const Recipe& margarita = recipeFor(PizzaType::Margarita);

    for (int i = 0; i < 5; ++i) CHECK(stock.tryConsume(margarita));

    std::atomic<bool> stop{false};
    std::atomic<bool> done{false};
    std::atomic<bool> result{true};
    std::thread waiter([&]() {
        result.store(stock.waitAndConsume(margarita, stop));
        done.store(true);
    });

    std::this_thread::sleep_for(80ms);
    CHECK_FALSE(done.load());

    stop.store(true);
    stock.notifyAll();

    auto deadline = std::chrono::steady_clock::now() + 500ms;
    while (!done.load() && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(10ms);
    CHECK(done.load());
    CHECK_FALSE(result.load());
    // Stop must not consume.
    CHECK(stock.available(Ingredient::Dough) == 0);
    waiter.join();
}

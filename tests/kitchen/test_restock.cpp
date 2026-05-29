/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_restock
*/

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "plazza/kitchen/Restocker.hpp"
#include "plazza/kitchen/Stock.hpp"
#include "plazza/pizza/Pizza.hpp"

using namespace plazza::kitchen;
using namespace plazza::pizza;
using namespace std::chrono_literals;

namespace {

    void drain(Stock& stock, Ingredient ing)
    {
        // Use a single-ingredient probe: a synthetic recipe that consumes one of ing.
        // We don't have one, so consume pizzas until counts reach 0 by inspection.
        // Simpler: tryConsume any recipe that touches ing until it fails or count==0.
        const Recipe& margarita = recipeFor(PizzaType::Margarita);
        const Recipe& americana = recipeFor(PizzaType::Americana);
        for (int i = 0; i < 10 && stock.available(ing) > 0; ++i) {
            if (!stock.tryConsume(margarita) && !stock.tryConsume(americana)) break;
        }
    }

}  // namespace

TEST_CASE("Restocker bumps stock periodically", "[restock]")
{
    Stock stock;
    Restocker restocker(stock, /*periodMs=*/50);

    // Drain Dough to 0 so we observe restocks accumulating from a known floor.
    drain(stock, Ingredient::Dough);
    CHECK(stock.available(Ingredient::Dough) == 0);

    restocker.start();
    std::this_thread::sleep_for(250ms);
    restocker.stop();

    // 250ms / 50ms ≈ 5 ticks; allow generous slack for scheduler jitter (>=3).
    CHECK(stock.available(Ingredient::Dough) >= 3);
}

TEST_CASE("Restocker::stop halts further restocks", "[restock]")
{
    Stock stock;
    Restocker restocker(stock, /*periodMs=*/40);

    drain(stock, Ingredient::Dough);
    restocker.start();
    std::this_thread::sleep_for(120ms);
    restocker.stop();

    auto frozen = stock.available(Ingredient::Dough);
    std::this_thread::sleep_for(200ms);
    CHECK(stock.available(Ingredient::Dough) == frozen);
}

TEST_CASE("Restocker wakes a parked waiter", "[restock][blocking]")
{
    Stock stock;
    Restocker restocker(stock, /*periodMs=*/40);

    const Recipe& margarita = recipeFor(PizzaType::Margarita);
    for (int i = 0; i < 5; ++i) CHECK(stock.tryConsume(margarita));
    CHECK(stock.available(Ingredient::Dough) == 0);

    std::atomic<bool> stop{false};
    std::atomic<bool> consumed{false};
    std::thread waiter([&]() { consumed.store(stock.waitAndConsume(margarita, stop)); });

    restocker.start();

    auto deadline = std::chrono::steady_clock::now() + 1s;
    while (!consumed.load() && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(10ms);
    CHECK(consumed.load());

    waiter.join();
    restocker.stop();
}

TEST_CASE("Restocker dtor stops thread without hanging", "[restock]")
{
    Stock stock;
    auto t0 = std::chrono::steady_clock::now();
    {
        Restocker restocker(stock, /*periodMs=*/500);
        restocker.start();
        std::this_thread::sleep_for(50ms);
    }
    auto elapsed = std::chrono::steady_clock::now() - t0;
    // Should not wait out the 500ms period — stop() wakes the CV.
    CHECK(elapsed < 400ms);
}

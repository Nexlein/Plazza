/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_cook_pool
*/

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>

#include "plazza/kitchen/Kitchen.hpp"
#include "plazza/pizza/Pizza.hpp"

using namespace plazza::kitchen;
using namespace plazza::pizza;
using namespace std::chrono_literals;

namespace {

    Kitchen::Config makeCfg(std::size_t cookCount, float multiplier)
    {
        // basePath is unused here — we drive tryAccept directly without
        // ever calling run(), so no FIFOs are created.
        return Kitchen::Config{cookCount, multiplier, /*restockMs=*/1000,
                               /*channelBasePath=*/"/tmp/plazza_unused"};
    }

}  // namespace

TEST_CASE("Kitchen::tryAccept respects 2 x N capacity gate", "[kitchen][cooks]")
{
    // N=3, multiplier 0.1 → Margarita (baseBakeTime=1s) sleeps ~100ms per cook,
    // long enough for the 7th tryAccept call to land while all six are in-flight.
    Kitchen k(makeCfg(3, 0.1F));

    std::atomic<int> completed{0};
    k.setOnComplete([&](const Pizza&, uint32_t, uint32_t) { completed.fetch_add(1); });

    const Pizza pizza = makePizza(PizzaType::Margarita, PizzaSize::M);

    int accepted = 0;
    for (int i = 0; i < 6; ++i) {
        if (k.tryAccept(pizza)) ++accepted;
    }
    CHECK(accepted == 6);
    CHECK(k.outstanding() == 6);

    // 7th must be rejected while cooks are still busy.
    CHECK(!k.tryAccept(pizza));

    // Wait for all six placeholder cooks to complete.
    auto deadline = std::chrono::steady_clock::now() + 5s;
    while (completed.load() < 6 && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(10ms);
    CHECK(completed.load() == 6);
    CHECK(k.outstanding() == 0);

    // Capacity must recover once cooks are free.
    CHECK(k.tryAccept(pizza));
    while (completed.load() < 7 && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(10ms);
    CHECK(completed.load() == 7);
}

TEST_CASE("Kitchen with N=1 rejects after 2 pizzas in flight", "[kitchen][cooks]")
{
    Kitchen k(makeCfg(1, 0.1F));
    std::atomic<int> completed{0};
    k.setOnComplete([&](const Pizza&, uint32_t, uint32_t) { completed.fetch_add(1); });

    const Pizza pizza = makePizza(PizzaType::Regina, PizzaSize::L);

    CHECK(k.tryAccept(pizza));
    CHECK(k.tryAccept(pizza));
    CHECK(!k.tryAccept(pizza));  // 2 * N reached

    auto deadline = std::chrono::steady_clock::now() + 5s;
    while (completed.load() < 2 && std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(10ms);
    CHECK(completed.load() == 2);
}

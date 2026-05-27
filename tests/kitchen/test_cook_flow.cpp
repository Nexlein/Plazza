/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_cook_flow
*/

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "plazza/pizza/Pizza.hpp"
#include "plazza/protocol/Frame.hpp"
#include "plazza/reception/KitchenHandle.hpp"

using namespace plazza::kitchen;
using namespace plazza::pizza;
using namespace plazza::protocol;
using namespace plazza::reception;
using namespace std::chrono_literals;

namespace {

    struct FrameSink {
        std::mutex mu;
        std::vector<AnyFrame> frames;

        void push(const AnyFrame& f)
        {
            std::lock_guard<std::mutex> lock(mu);
            frames.push_back(f);
        }

        std::vector<AnyFrame> snapshot()
        {
            std::lock_guard<std::mutex> lock(mu);
            return frames;
        }
    };

    bool waitUntil(
        FrameSink& sink, std::chrono::steady_clock::duration timeout,
        const std::function<bool(const std::vector<AnyFrame>&)>& predicate)
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (predicate(sink.snapshot())) return true;
            std::this_thread::sleep_for(10ms);
        }
        return false;
    }

    int countCompleted(const std::vector<AnyFrame>& frames)
    {
        int n = 0;
        for (const auto& f : frames)
            if (std::holds_alternative<CompletedFrame>(f)) ++n;
        return n;
    }

    int countAccepted(const std::vector<AnyFrame>& frames)
    {
        int n = 0;
        for (const auto& f : frames)
            if (std::holds_alternative<AcceptedFrame>(f)) ++n;
        return n;
    }

}  // namespace

TEST_CASE("Cook flow: real bake time honours multiplier", "[kitchen][cook][timing]")
{
    const std::string base = "/tmp/plazza_test_cook_timing";
    // Regina has baseBakeTime=2; with multiplier=0.5 we expect ~1000ms.
    // restockMs=0 keeps the restocker off so timing is purely the cook.
    Kitchen::Config cfg{
        /*cookCount=*/2, /*cookingMultiplier=*/0.5F, /*restockMs=*/0, base};

    FrameSink sink;
    {
        KitchenHandle handle(cfg, /*id=*/0);
        handle.onFrame([&](const AnyFrame& f) { sink.push(f); });

        auto sent = std::chrono::steady_clock::now();
        handle.sendFrame(RequestFrame{
            /*orderId=*/1, /*pizzaIdx=*/0, PizzaType::Regina, PizzaSize::M});

        REQUIRE(waitUntil(sink, 3s, [](const auto& frames) {
            return countCompleted(frames) == 1;
        }));

        auto elapsed = std::chrono::steady_clock::now() - sent;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        CHECK(ms >= 800);
        CHECK(ms <= 1800);
    }
}

TEST_CASE("Cook flow: stock back-pressure resolves via restocker",
          "[kitchen][cook][stock]")
{
    const std::string base = "/tmp/plazza_test_cook_backpressure";
    // 6 Reginas, each needs Ham. Initial Ham=5, so one cook must wait
    // for the restocker (period=80ms) before it can proceed.
    Kitchen::Config cfg{
        /*cookCount=*/4, /*cookingMultiplier=*/0.05F, /*restockMs=*/80, base};

    FrameSink sink;
    {
        KitchenHandle handle(cfg, /*id=*/0);
        handle.onFrame([&](const AnyFrame& f) { sink.push(f); });

        for (uint32_t idx = 0; idx < 6; ++idx)
            handle.sendFrame(RequestFrame{
                /*orderId=*/7, /*pizzaIdx=*/idx, PizzaType::Regina, PizzaSize::M});

        REQUIRE(waitUntil(sink, 5s, [](const auto& frames) {
            return countAccepted(frames) == 6 && countCompleted(frames) == 6;
        }));
    }

    auto frames = sink.snapshot();
    CHECK(countAccepted(frames) == 6);
    CHECK(countCompleted(frames) == 6);
}

TEST_CASE("Cook flow: shutdown wakes cooks parked on missing ingredients",
          "[kitchen][cook][shutdown]")
{
    const std::string base = "/tmp/plazza_test_cook_shutdown";
    // 8 Reginas saturate the 2*N=8 capacity; 8 cooks need Ham but only 5
    // initial units exist. With restockMs=0 the last 3 cooks would park
    // forever — Kitchen::~Kitchen must wake them via Stock::notifyAll.
    Kitchen::Config cfg{
        /*cookCount=*/4, /*cookingMultiplier=*/0.0F, /*restockMs=*/0, base};

    FrameSink sink;
    auto t0 = std::chrono::steady_clock::now();
    {
        KitchenHandle handle(cfg, /*id=*/0);
        handle.onFrame([&](const AnyFrame& f) { sink.push(f); });

        for (uint32_t idx = 0; idx < 8; ++idx)
            handle.sendFrame(RequestFrame{
                /*orderId=*/3, /*pizzaIdx=*/idx, PizzaType::Regina, PizzaSize::L});

        // Wait until the kitchen has acked all 8 and completed the 5 that
        // could actually get ingredients. The remaining 3 are now parked.
        REQUIRE(waitUntil(sink, 3s, [](const auto& frames) {
            return countAccepted(frames) == 8 && countCompleted(frames) >= 5;
        }));
    }
    auto elapsed = std::chrono::steady_clock::now() - t0;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    // The handle shutdown (closeWrite → child exit → process->wait) must
    // not block on the parked cooks. Generous budget covers the 3s waitUntil
    // plus the actual teardown — teardown itself should be near-instant.
    CHECK(ms < 4500);
}

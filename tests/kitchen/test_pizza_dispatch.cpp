/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_dispatch_balance
*/

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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

    bool waitUntil(FrameSink& sink, std::chrono::steady_clock::duration timeout,
                   const std::function<bool(const std::vector<AnyFrame>&)>& predicate)
    {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (predicate(sink.snapshot())) return true;
            std::this_thread::sleep_for(10ms);
        }
        return false;
    }

    int countAccepted(const std::vector<AnyFrame>& frames)
    {
        int n = 0;
        for (const auto& f : frames)
            if (std::holds_alternative<AcceptedFrame>(f)) ++n;
        return n;
    }

}  // namespace

TEST_CASE("Dispatch: 9 pizzas across 3 kitchens are balanced within ±1", "[dispatch][balance]")
{
    // 3 kitchens, cooks=3 → cap=6 each
    // On envoie 3 pizzas par kitchen manuellement pour simuler le dispatch balancé
    const std::size_t cooks = 3;
    std::vector<std::string> bases = {"/tmp/plazza_test_balance_0", "/tmp/plazza_test_balance_1",
                                      "/tmp/plazza_test_balance_2"};

    Kitchen::Config cfg0{cooks, 0.05F, 0, bases[0]};
    Kitchen::Config cfg1{cooks, 0.05F, 0, bases[1]};
    Kitchen::Config cfg2{cooks, 0.05F, 0, bases[2]};

    std::vector<FrameSink> sinks(3);

    KitchenHandle h0(cfg0, 0);
    KitchenHandle h1(cfg1, 1);
    KitchenHandle h2(cfg2, 2);

    h0.onFrame([&](const AnyFrame& f) { sinks[0].push(f); });
    h1.onFrame([&](const AnyFrame& f) { sinks[1].push(f); });
    h2.onFrame([&](const AnyFrame& f) { sinks[2].push(f); });

    // Dispatch balancé : 3 pizzas par kitchen = 9 total
    for (uint32_t idx = 0; idx < 3; ++idx)
        h0.sendFrame(RequestFrame{1, idx, PizzaType::Margarita, PizzaSize::M});
    for (uint32_t idx = 0; idx < 3; ++idx)
        h1.sendFrame(RequestFrame{2, idx, PizzaType::Margarita, PizzaSize::M});
    for (uint32_t idx = 0; idx < 3; ++idx)
        h2.sendFrame(RequestFrame{3, idx, PizzaType::Margarita, PizzaSize::M});

    // Attendre que les 9 soient acceptées
    FrameSink combined;
    auto cond = [&](const std::vector<AnyFrame>&) {
        return countAccepted(sinks[0].snapshot()) +
               countAccepted(sinks[1].snapshot()) +
               countAccepted(sinks[2].snapshot()) == 9;
    };
    REQUIRE(waitUntil(combined, 3s, cond));

    // Distribution within ±1 : max - min <= 1
    int a0 = countAccepted(sinks[0].snapshot());
    int a1 = countAccepted(sinks[1].snapshot());
    int a2 = countAccepted(sinks[2].snapshot());
    int mn = std::min({a0, a1, a2});
    int mx = std::max({a0, a1, a2});
    CHECK(mx - mn <= 1);
}

TEST_CASE("Dispatch: saturated kitchen does not block others", "[dispatch][saturation]")
{
    // kitchen 0 saturée (cap=6, on envoie 6), kitchen 1 libre
    // kitchen 1 doit pouvoir accepter sans attendre kitchen 0
    const std::size_t cooks = 3;

    FrameSink sink0, sink1;
    KitchenHandle h0({cooks, 0.05F, 0, "/tmp/plazza_test_sat_0"}, 0);
    KitchenHandle h1({cooks, 0.05F, 0, "/tmp/plazza_test_sat_1"}, 1);

    h0.onFrame([&](const AnyFrame& f) { sink0.push(f); });
    h1.onFrame([&](const AnyFrame& f) { sink1.push(f); });

    // Saturer kitchen 0
    for (uint32_t idx = 0; idx < cooks * 2; ++idx)
        h0.sendFrame(RequestFrame{1, idx, PizzaType::Margarita, PizzaSize::M});

    // Envoyer à kitchen 1
    h1.sendFrame(RequestFrame{2, 0, PizzaType::Margarita, PizzaSize::M});

    // kitchen 1 doit accepter rapidement, indépendamment de kitchen 0
    auto cond = [](const std::vector<AnyFrame>& frames) { return countAccepted(frames) == 1; };
    REQUIRE(waitUntil(sink1, 2s, cond));
}
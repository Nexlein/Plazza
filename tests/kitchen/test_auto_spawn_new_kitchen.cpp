/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_dispatch_balance
*/

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

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

    int countAccepted(const std::vector<AnyFrame>& frames)
    {
        int n = 0;
        for (const auto& f : frames)
            if (std::holds_alternative<AcceptedFrame>(f)) ++n;
        return n;
    }

}  // namespace

TEST_CASE("Dispatch: saturated kitchen triggers exactly one new kitchen spawn",
          "[dispatch][spawn]")
{
    std::system("rm -f /tmp/plazza_test_nospawn_0* /tmp/plazza_test_nospawn_1*");
    // cooks=3 → cap=6 ; on sature kitchen 0 avec 6 pizzas puis on envoie
    // une 7ème : elle doit partir sur une nouvelle kitchen (kitchen 1).
    const std::size_t cooks = 3;

    KitchenHandle h0({cooks, 0.05F, 0, "/tmp/plazza_test_spawn_0"}, 0);
    KitchenHandle h1({cooks, 0.05F, 0, "/tmp/plazza_test_spawn_1"}, 1);

    FrameSink sink0, sink1;
    h0.onFrame([&](const AnyFrame& f) { sink0.push(f); });
    h1.onFrame([&](const AnyFrame& f) { sink1.push(f); });

    // Saturer kitchen 0 : 2 * cooks = 6 pizzas
    for (uint32_t idx = 0; idx < cooks * 2; ++idx)
        h0.sendFrame(RequestFrame{1, idx, PizzaType::Margarita, PizzaSize::M});

    // Attendre que kitchen 0 ait bien accepté les 6
    REQUIRE(waitUntil(sink0, 3s, [](const auto& frames) {
        return countAccepted(frames) == 6;
    }));

    // Envoyer la 7ème pizza sur kitchen 1 (nouvelle kitchen spawnée)
    h1.sendFrame(RequestFrame{2, 0, PizzaType::Margarita, PizzaSize::M});

    // kitchen 1 doit accepter la pizza
    REQUIRE(waitUntil(sink1, 2s, [](const auto& frames) {
        return countAccepted(frames) == 1;
    }));

    // kitchen 0 ne doit pas avoir reçu de pizza supplémentaire
    CHECK(countAccepted(sink0.snapshot()) == 6);
}

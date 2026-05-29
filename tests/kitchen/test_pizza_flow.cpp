/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_pizza_flow
*/

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

    struct Tally {
        int accepted = 0;
        int rejected = 0;
        int completed = 0;
        int statusReply = 0;
    };

    Tally tally(const std::vector<AnyFrame>& frames)
    {
        Tally t;
        for (const auto& f : frames) {
            if (std::holds_alternative<AcceptedFrame>(f))
                ++t.accepted;
            else if (std::holds_alternative<RejectedFrame>(f))
                ++t.rejected;
            else if (std::holds_alternative<CompletedFrame>(f))
                ++t.completed;
            else if (std::holds_alternative<StatusReplyFrame>(f))
                ++t.statusReply;
        }
        return t;
    }

}  // namespace

TEST_CASE("Pizza flow: 3 requests → 3 Accepted + 3 Completed", "[kitchen][protocol]")
{
    const std::string base = "/tmp/plazza_test_pizza_flow";
    Kitchen::Config cfg{/*cookCount=*/2, /*cookingMultiplier=*/0.05F, /*restockMs=*/1000, base};

    FrameSink sink;

    {
        KitchenHandle handle(cfg, /*id=*/0);
        handle.onFrame([&](const AnyFrame& f) { sink.push(f); });

        for (uint32_t idx = 0; idx < 3; ++idx)
            handle.sendFrame(
                RequestFrame{/*orderId=*/1, /*pizzaIdx=*/idx, PizzaType::Margarita, PizzaSize::M});

        auto condition = [](const std::vector<AnyFrame>& frames) {
            auto t = tally(frames);
            return t.accepted == 3 && t.completed == 3;
        };
        REQUIRE(waitUntil(sink, 5s, condition));
    }

    auto t = tally(sink.snapshot());
    CHECK(t.accepted == 3);
    CHECK(t.rejected == 0);
    CHECK(t.completed == 3);
}

TEST_CASE("Pizza flow: oversubscribed request is Rejected", "[kitchen][protocol]")
{
    const std::string base = "/tmp/plazza_test_pizza_reject";
    Kitchen::Config cfg{/*cookCount=*/1, /*cookingMultiplier=*/0.2F, /*restockMs=*/1000, base};

    FrameSink sink;

    {
        KitchenHandle handle(cfg, /*id=*/0);
        handle.onFrame([&](const AnyFrame& f) { sink.push(f); });

        for (uint32_t idx = 0; idx < 3; ++idx)
            handle.sendFrame(
                RequestFrame{/*orderId=*/9, /*pizzaIdx=*/idx, PizzaType::Margarita, PizzaSize::S});

        auto condition = [](const std::vector<AnyFrame>& frames) {
            auto t = tally(frames);
            return (t.accepted + t.rejected) == 3 && t.completed == 2;
        };
        REQUIRE(waitUntil(sink, 5s, condition));
    }

    auto t = tally(sink.snapshot());
    CHECK(t.accepted == 2);
    CHECK(t.rejected == 1);
    CHECK(t.completed == 2);
}

TEST_CASE("Pizza flow: StatusQuery replies with cook count and outstanding", "[kitchen][protocol]")
{
    const std::string base = "/tmp/plazza_test_pizza_status";
    Kitchen::Config cfg{/*cookCount=*/3, /*cookingMultiplier=*/0.05F, /*restockMs=*/1000, base};

    FrameSink sink;

    {
        KitchenHandle handle(cfg, /*id=*/0);
        handle.onFrame([&](const AnyFrame& f) { sink.push(f); });

        handle.sendFrame(StatusQueryFrame{});

        auto condition = [](const std::vector<AnyFrame>& frames) { return !frames.empty(); };
        REQUIRE(waitUntil(sink, 2s, condition));
    }

    auto frames = sink.snapshot();
    REQUIRE(!frames.empty());
    REQUIRE(std::holds_alternative<StatusReplyFrame>(frames.front()));
    const auto& reply = std::get<StatusReplyFrame>(frames.front());
    CHECK(reply.cooksTotal == 3);
    CHECK(reply.outstanding == 0);
    for (std::size_t i = 0; i < 9; ++i) CHECK(reply.stock[i] >= 5);
}

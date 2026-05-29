/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_frames
*/

#include <catch2/catch_test_macros.hpp>

#include "plazza/core/SerialBuffer.hpp"
#include "plazza/pizza/Pizza.hpp"
#include "plazza/protocol/Frame.hpp"

using namespace plazza::core;
using namespace plazza::pizza;
using namespace plazza::protocol;

TEST_CASE("Frame: Heartbeat round-trip", "[protocol]")
{
    auto bytes = encode(HeartbeatFrame{0xDEADBEEFu});
    auto frame = decode(bytes);
    REQUIRE(std::holds_alternative<HeartbeatFrame>(frame));
    CHECK(std::get<HeartbeatFrame>(frame).token == 0xDEADBEEFu);
}

TEST_CASE("Frame: Ack round-trip", "[protocol]")
{
    auto bytes = encode(AckFrame{42});
    auto frame = decode(bytes);
    REQUIRE(std::holds_alternative<AckFrame>(frame));
    CHECK(std::get<AckFrame>(frame).token == 42);
}

TEST_CASE("Frame: Request round-trip", "[protocol]")
{
    RequestFrame in{/*orderId=*/7, /*pizzaIdx=*/2, PizzaType::Regina, PizzaSize::XL};
    auto bytes = encode(in);
    auto frame = decode(bytes);
    REQUIRE(std::holds_alternative<RequestFrame>(frame));
    const auto& out = std::get<RequestFrame>(frame);
    CHECK(out.orderId == in.orderId);
    CHECK(out.pizzaIdx == in.pizzaIdx);
    CHECK(out.type == in.type);
    CHECK(out.size == in.size);
}

TEST_CASE("Frame: Request covers every PizzaType and PizzaSize", "[protocol]")
{
    for (auto t :
         {PizzaType::Regina, PizzaType::Margarita, PizzaType::Americana, PizzaType::Fantasia}) {
        for (auto s : {PizzaSize::S, PizzaSize::M, PizzaSize::L, PizzaSize::XL, PizzaSize::XXL}) {
            auto frame = decode(encode(RequestFrame{1, 0, t, s}));
            REQUIRE(std::holds_alternative<RequestFrame>(frame));
            const auto& out = std::get<RequestFrame>(frame);
            CHECK(out.type == t);
            CHECK(out.size == s);
        }
    }
}

TEST_CASE("Frame: Accepted / Rejected / Completed round-trip", "[protocol]")
{
    {
        auto frame = decode(encode(AcceptedFrame{11, 22}));
        REQUIRE(std::holds_alternative<AcceptedFrame>(frame));
        CHECK(std::get<AcceptedFrame>(frame).orderId == 11);
        CHECK(std::get<AcceptedFrame>(frame).pizzaIdx == 22);
    }
    {
        auto frame = decode(encode(RejectedFrame{33, 44}));
        REQUIRE(std::holds_alternative<RejectedFrame>(frame));
        CHECK(std::get<RejectedFrame>(frame).orderId == 33);
        CHECK(std::get<RejectedFrame>(frame).pizzaIdx == 44);
    }
    {
        auto frame = decode(encode(CompletedFrame{55, 66}));
        REQUIRE(std::holds_alternative<CompletedFrame>(frame));
        CHECK(std::get<CompletedFrame>(frame).orderId == 55);
        CHECK(std::get<CompletedFrame>(frame).pizzaIdx == 66);
    }
}

TEST_CASE("Frame: StatusQuery / StatusReply round-trip", "[protocol]")
{
    auto qframe = decode(encode(StatusQueryFrame{}));
    REQUIRE(std::holds_alternative<StatusQueryFrame>(qframe));

    auto rframe = decode(encode(StatusReplyFrame{4, 3}));
    REQUIRE(std::holds_alternative<StatusReplyFrame>(rframe));
    CHECK(std::get<StatusReplyFrame>(rframe).cooksTotal == 4);
    CHECK(std::get<StatusReplyFrame>(rframe).outstanding == 3);
}

TEST_CASE("Frame: peekKind reads the kind byte", "[protocol]")
{
    CHECK(peekKind(encode(HeartbeatFrame{})) == FrameKind::Heartbeat);
    CHECK(peekKind(encode(AckFrame{})) == FrameKind::Ack);
    CHECK(peekKind(encode(RequestFrame{})) == FrameKind::Request);
    CHECK(peekKind(encode(CompletedFrame{})) == FrameKind::Completed);
    CHECK(peekKind(encode(StatusReplyFrame{})) == FrameKind::StatusReply);
}

TEST_CASE("SerialBuffer: std::size_t round-trip", "[protocol][serial]")
{
    SerialBuffer buf;
    std::size_t in = 0xFFFFFFFFFFFFFFFFull;
    buf << in;
    std::size_t out = 0;
    buf >> out;
    CHECK(out == in);
}

TEST_CASE("SerialBuffer: std::string round-trip including empty", "[protocol][serial]")
{
    {
        SerialBuffer buf;
        std::string in = "hello kitchen";
        buf << in;
        std::string out;
        buf >> out;
        CHECK(out == in);
    }
    {
        SerialBuffer buf;
        std::string in;
        buf << in;
        std::string out = "stale";
        buf >> out;
        CHECK(out.empty());
    }
}

TEST_CASE("SerialBuffer: Pizza round-trip via memcpy of trivially-copyable type",
          "[protocol][serial]")
{
    SerialBuffer buf;
    Pizza in = makePizza(PizzaType::Fantasia, PizzaSize::XXL);
    buf << in;
    Pizza out{};
    buf >> out;
    CHECK(out.type == in.type);
    CHECK(out.size == in.size);
    CHECK(out.recipe.ingredientCount == in.recipe.ingredientCount);
    CHECK(out.recipe.baseBakeTime == in.recipe.baseBakeTime);
    for (std::size_t i = 0; i < out.recipe.ingredientCount; ++i)
        CHECK(out.recipe.ingredients[i] == in.recipe.ingredients[i]);
}

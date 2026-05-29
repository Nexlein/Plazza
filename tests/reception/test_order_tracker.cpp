/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_order_tracker
*/

#include <string>

#include <catch2/catch_test_macros.hpp>

#include "plazza/pizza/PizzaSize.hpp"
#include "plazza/pizza/PizzaType.hpp"
#include "plazza/reception/OrderTracker.hpp"

using plazza::pizza::PizzaSize;
using plazza::pizza::PizzaType;
using plazza::reception::OrderTracker;

namespace {

    bool contains(const std::string& haystack, const std::string& needle)
    {
        return haystack.find(needle) != std::string::npos;
    }

}  // namespace

TEST_CASE("OrderTracker: single-pizza order completes immediately", "[reception][order]")
{
    OrderTracker tracker;
    const auto id = tracker.add(1, PizzaType::Regina, PizzaSize::XXL);
    REQUIRE(tracker.pending() == 1);

    const auto update = tracker.complete(id);
    CHECK(update.known);
    CHECK(update.orderComplete);
    CHECK(contains(update.pizzaLine, "order #" + std::to_string(id)));
    CHECK(contains(update.pizzaLine, "regina"));
    CHECK(contains(update.pizzaLine, "XXL"));
    CHECK(contains(update.pizzaLine, "(1/1)"));
    CHECK(contains(update.orderLine, "complete"));
    CHECK(tracker.pending() == 0);
}

TEST_CASE("OrderTracker: multi-pizza order reports progress then completes", "[reception][order]")
{
    OrderTracker tracker;
    const auto id = tracker.add(3, PizzaType::Fantasia, PizzaSize::M);

    const auto first = tracker.complete(id);
    CHECK(first.known);
    CHECK_FALSE(first.orderComplete);
    CHECK(contains(first.pizzaLine, "(1/3)"));
    CHECK(first.orderLine.empty());

    const auto second = tracker.complete(id);
    CHECK_FALSE(second.orderComplete);
    CHECK(contains(second.pizzaLine, "(2/3)"));

    const auto third = tracker.complete(id);
    CHECK(third.orderComplete);
    CHECK(contains(third.pizzaLine, "(3/3)"));
    CHECK(contains(third.orderLine, "order #" + std::to_string(id)));
    CHECK(contains(third.orderLine, "complete"));
    CHECK(tracker.pending() == 0);
}

TEST_CASE("OrderTracker: unknown order id yields no message", "[reception][order]")
{
    OrderTracker tracker;
    const auto update = tracker.complete(999);
    CHECK_FALSE(update.known);
    CHECK(update.pizzaLine.empty());
    CHECK_FALSE(update.orderComplete);
}

TEST_CASE("OrderTracker: ids are monotonic and orders are independent", "[reception][order]")
{
    OrderTracker tracker;
    const auto a = tracker.add(2, PizzaType::Regina, PizzaSize::S);
    const auto b = tracker.add(1, PizzaType::Margarita, PizzaSize::L);
    CHECK(b > a);
    REQUIRE(tracker.pending() == 2);

    // Finishing order b must not affect order a.
    const auto doneB = tracker.complete(b);
    CHECK(doneB.orderComplete);
    CHECK(tracker.pending() == 1);

    const auto firstA = tracker.complete(a);
    CHECK_FALSE(firstA.orderComplete);
    const auto secondA = tracker.complete(a);
    CHECK(secondA.orderComplete);
    CHECK(tracker.pending() == 0);
}

TEST_CASE("OrderTracker: stray completion after done is ignored", "[reception][order]")
{
    OrderTracker tracker;
    const auto id = tracker.add(1, PizzaType::Americana, PizzaSize::XL);
    const auto first = tracker.complete(id);
    CHECK(first.orderComplete);
    // The order has been forgotten; a duplicate frame is harmless.
    const auto stray = tracker.complete(id);
    CHECK_FALSE(stray.known);
}

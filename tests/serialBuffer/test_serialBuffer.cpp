/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** test_serial_buffer
*/

#include <catch2/catch_test_macros.hpp>

#include "plazza/core/SerialBuffer.hpp"

using namespace plazza::core;
using namespace plazza::reception;
using namespace plazza::pizza;

TEST_CASE("SerialBuffer round-trip Order") {
    SerialBuffer buf;
    Order original{PizzaType::Regina, PizzaSize::XL, 3};
    buf << original;
    Order received;
    buf >> received;

    CHECK(received.type == original.type);
    CHECK(received.size == original.size);
    CHECK(received.count == original.count);
}

TEST_CASE("SerialBuffer round-trip multiple Orders") {
    SerialBuffer buf;
    Order o1{PizzaType::Regina, PizzaSize::XL, 3};
    Order o2{PizzaType::Margarita, PizzaSize::S, 1};
    buf << o1 << o2;

    Order r1, r2;
    buf >> r1 >> r2;

    CHECK(r1.type == o1.type);
    CHECK(r2.type == o2.type);
    CHECK(r2.count == o2.count);
}

TEST_CASE("SerialBuffer count boundary") {
    SerialBuffer buf;
    Order original{PizzaType::Fantasia, PizzaSize::XXL, std::numeric_limits<std::size_t>::max()};
    buf << original;
    Order received;
    buf >> received;

    CHECK(received.count == original.count);
}

TEST_CASE("SerialBuffer all PizzaTypes round-trip") {
    for (auto type : {PizzaType::Regina, PizzaType::Margarita, PizzaType::Americana, PizzaType::Fantasia}) {
        SerialBuffer buf;
        Order original{type, PizzaSize::M, 2};
        buf << original;
        Order received;
        buf >> received;

        CHECK(received.type == type);
    }
}
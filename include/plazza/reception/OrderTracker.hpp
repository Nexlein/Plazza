/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** OrderTracker
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include "plazza/core/Mutex.hpp"
#include "plazza/pizza/PizzaSize.hpp"
#include "plazza/pizza/PizzaType.hpp"

namespace plazza::reception {

    // Tracks per-order completion so the reception can tell the user when each
    // pizza of an order is done and when the whole order is complete. Every
    // method is safe to call concurrently from the kitchen reader threads.
    class OrderTracker {
        public:
        // Messages to emit for a single completed pizza. `known` is false when
        // the order id is not (or no longer) registered.
        struct Update {
            bool known{false};
            std::string pizzaLine;
            bool orderComplete{false};
            std::string orderLine;
        };

        // Registers a new order of `count` pizzas and returns its stable,
        // monotonically increasing id.
        uint32_t add(std::size_t count, plazza::pizza::PizzaType type,
                     plazza::pizza::PizzaSize size);

        // Records one finished pizza of `orderId` and returns the lines to show.
        // The order is forgotten once its last pizza is done.
        Update complete(uint32_t orderId);

        // Number of orders still awaiting completion.
        std::size_t pending() const;

        private:
        struct Entry {
            std::size_t total{0};
            std::size_t done{0};
            plazza::pizza::PizzaType type{};
            plazza::pizza::PizzaSize size{};
        };

        mutable core::Mutex _mutex;
        std::map<uint32_t, Entry> _orders;
        uint32_t _nextId{0};
    };

}  // namespace plazza::reception

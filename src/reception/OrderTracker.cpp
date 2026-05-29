/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** OrderTracker
*/

#include "plazza/reception/OrderTracker.hpp"

#include <sstream>

#include "plazza/core/ScopedLock.hpp"

namespace plazza::reception {

    uint32_t OrderTracker::add(std::size_t count, plazza::pizza::PizzaType type,
                               plazza::pizza::PizzaSize size)
    {
        core::ScopedLock lock(&_mutex);
        const uint32_t id = ++_nextId;
        _orders.emplace(id, Entry{count, 0, type, size});
        return id;
    }

    OrderTracker::Update OrderTracker::complete(uint32_t orderId)
    {
        core::ScopedLock lock(&_mutex);
        Update update;

        auto it = _orders.find(orderId);
        if (it == _orders.end()) return update;
        update.known = true;

        Entry& entry = it->second;
        if (entry.done < entry.total) ++entry.done;

        std::ostringstream pizza;
        pizza << "order #" << orderId << ": " << plazza::pizza::typeToString(entry.type) << " "
              << plazza::pizza::sizeToString(entry.size) << " done (" << entry.done << "/"
              << entry.total << ")";
        update.pizzaLine = pizza.str();

        if (entry.done >= entry.total) {
            std::ostringstream order;
            order << "order #" << orderId << " complete (" << entry.total << " pizza(s))";
            update.orderLine = order.str();
            update.orderComplete = true;
            _orders.erase(it);
        }
        return update;
    }

    std::size_t OrderTracker::pending() const
    {
        core::ScopedLock lock(&_mutex);
        return _orders.size();
    }

}  // namespace plazza::reception

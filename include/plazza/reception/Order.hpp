/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Order
*/

#pragma once

#include <cstddef>

#include "plazza/pizza/PizzaSize.hpp"
#include "plazza/pizza/PizzaType.hpp"

namespace plazza::reception {

    struct Order {
        plazza::pizza::PizzaType type;
        plazza::pizza::PizzaSize size;
        std::size_t count;
    };

}  // namespace plazza::reception

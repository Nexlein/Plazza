/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Parser
*/

#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "plazza/reception/Order.hpp"

namespace plazza::reception {

    class Parser {
        public:
        static std::optional<std::vector<Order>> parse(std::string_view input);
    };

}  // namespace plazza::reception

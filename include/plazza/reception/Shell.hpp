/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Shell
*/

#pragma once

#include <iosfwd>
#include <queue>

#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Order.hpp"

namespace plazza::reception {

    class Shell {
        public:
        Shell(Logger& logger, std::istream& in, std::ostream& out);

        int run();

        private:
        Logger& _logger;
        std::istream& _in;
        std::ostream& _out;
        std::queue<Order> _pending;

        void handleStatus();
        bool handleOrderLine(const std::string& line);
    };

}  // namespace plazza::reception

/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Shell
*/

#pragma once

#include <functional>
#include <iosfwd>
#include <queue>

#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Order.hpp"

namespace plazza::reception {

    class KitchenHandle;

    class Shell {
        public:
        Shell(Logger& logger, std::istream& in, std::ostream& out,
              std::function<void(const Order&)> onOrder,
              std::function<std::vector<KitchenHandle*>()> getKitchens);

        int run();

        private:
        Logger& _logger;
        std::istream& _in;
        std::ostream& _out;
        std::function<void(const Order&)> _onOrder;
        std::function<std::vector<KitchenHandle*>()> _getKitchens;

        void handleStatus();
        bool handleOrderLine(const std::string& line);
    };

}  // namespace plazza::reception

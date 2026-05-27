/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Cook
*/

#pragma once

#include <atomic>

#include "plazza/kitchen/Stock.hpp"
#include "plazza/pizza/Pizza.hpp"

namespace plazza::kitchen {

    class Cook {
        public:
        Cook(plazza::pizza::Pizza pizza, float multiplier, Stock& stock,
             const std::atomic<bool>& stop);

        void operator()();

        const plazza::pizza::Pizza& pizza() const { return _pizza; }

        private:
        plazza::pizza::Pizza _pizza;
        float _multiplier;
        Stock& _stock;
        const std::atomic<bool>& _stop;
    };

}  // namespace plazza::kitchen

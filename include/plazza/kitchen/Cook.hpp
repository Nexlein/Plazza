/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Cook
*/

#pragma once

#include <atomic>
#include <functional>

#include "plazza/kitchen/Stock.hpp"
#include "plazza/pizza/Pizza.hpp"

namespace plazza::kitchen {

    class Cook {
        public:
        Cook(plazza::pizza::Pizza pizza, float multiplier, Stock& stock,
             const std::atomic<bool>& stop);

        void operator()();

        const plazza::pizza::Pizza& pizza() const { return _pizza; }

        // Invoked once the ingredients are consumed and baking actually starts,
        // with the bake duration in milliseconds. Lets the kitchen mark the
        // owning cook slot as Baking for the graphical status report.
        void setOnBakeStart(std::function<void(long long)> fn) { _onBakeStart = std::move(fn); }

        private:
        plazza::pizza::Pizza _pizza;
        float _multiplier;
        Stock& _stock;
        const std::atomic<bool>& _stop;
        std::function<void(long long)> _onBakeStart;
    };

}  // namespace plazza::kitchen

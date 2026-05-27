/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Cook
*/

#include "plazza/kitchen/Cook.hpp"

#include <chrono>
#include <thread>

namespace plazza::kitchen {

    Cook::Cook(plazza::pizza::Pizza pizza, float multiplier, Stock& stock,
               const std::atomic<bool>& stop)
        : _pizza(pizza), _multiplier(multiplier), _stock(stock), _stop(stop)
    {
    }

    void Cook::operator()()
    {
        if (!_stock.waitAndConsume(_pizza.recipe, _stop)) return;
        long long ms = static_cast<long long>(_pizza.recipe.baseBakeTime * 1000.0F * _multiplier);
        if (ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

}  // namespace plazza::kitchen

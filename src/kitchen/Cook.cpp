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

    Cook::Cook(plazza::pizza::Pizza pizza, float multiplier)
        : _pizza(pizza), _multiplier(multiplier)
    {
    }

    void Cook::operator()()
    {
        // Slice 4.2 placeholder: sleep for the effective bake time. Slice 5.3
        // replaces this with ingredient consumption + real cooking.
        long long ms =
            static_cast<long long>(_pizza.recipe.baseBakeTime * 1000.0F * _multiplier);
        if (ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

}  // namespace plazza::kitchen

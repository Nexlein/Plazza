/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Cook
*/

#pragma once

#include "plazza/pizza/Pizza.hpp"

namespace plazza::kitchen {

    class Cook {
        public:
        Cook(plazza::pizza::Pizza pizza, float multiplier);

        void operator()();

        const plazza::pizza::Pizza& pizza() const { return _pizza; }

        private:
        plazza::pizza::Pizza _pizza;
        float _multiplier;
    };

}  // namespace plazza::kitchen

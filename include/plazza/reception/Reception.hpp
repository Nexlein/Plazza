/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Reception
*/

#pragma once

#include <cstddef>
#include <string>

namespace plazza::reception {

    class Reception {
        public:
        Reception(float cookingMultiplier, std::size_t cooksPerKitchen, std::size_t restockMs);

        int run();

        private:
        float _cookingMultiplier;
        std::size_t _cooksPerKitchen;
        std::size_t _restockMs;
        std::string _logPath = "plazza.log";
    };

}  // namespace plazza::reception

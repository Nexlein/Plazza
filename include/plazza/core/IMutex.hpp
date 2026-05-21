/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** IMutex
*/

#pragma once

namespace plazza::core {

    class IMutex {
        public:
        virtual ~IMutex() = default;

        virtual void lock() = 0;
        virtual void unlock() = 0;
        virtual void trylock() = 0;
    };

}  // namespace plazza::core

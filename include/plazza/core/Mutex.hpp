/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Mutex
*/

#pragma once

#include <pthread.h>

#include "plazza/core/IMutex.hpp"

namespace plazza::core {

    class Mutex : public IMutex {
        public:
        Mutex();
        ~Mutex() override;

        void lock() override;
        void unlock() override;
        void trylock() override;

        private:
        pthread_mutex_t _mutex;
    };

}  // namespace plazza::core

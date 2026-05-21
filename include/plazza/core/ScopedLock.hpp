/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** ScopedLock
*/

#pragma once

#include "plazza/core/IMutex.hpp"

namespace plazza::core {

    class ScopedLock {
        public:
        ScopedLock(IMutex* mutex);
        ~ScopedLock();

        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;

        private:
        IMutex* _mutex;
    };

}  // namespace plazza::core

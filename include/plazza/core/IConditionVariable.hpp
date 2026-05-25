/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** IConditionVariable
*/

#pragma once

#include "IMutex.hpp"
#include <chrono>

namespace plazza::core {

    class IConditionVariable {
        public:
        virtual ~IConditionVariable() = default;

        virtual void wait(plazza::core::IMutex &mutex) = 0;
        virtual void waitFor(plazza::core::IMutex& mutex, std::chrono::milliseconds duration) = 0;
        virtual void notifyOne() = 0;
        virtual void notifyAll() = 0;
    };

}  // namespace plazza::core

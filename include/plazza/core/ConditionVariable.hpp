/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** ConditionVariable
*/

#pragma once

#include "IConditionVariable.hpp"

#include <pthread.h>

namespace plazza::core {

    class ConditionVariable : public IConditionVariable {
        public:
        ConditionVariable();
        ~ConditionVariable() = default;

        void wait(plazza::core::IMutex &mutex) override;
        void waitFor(plazza::core::IMutex& mutex, std::chrono::milliseconds duration) override;
        void notifyOne() override;
        void notifyAll() override;

        private:
        pthread_cond_t _cond;
    };

}  // namespace plazza::core
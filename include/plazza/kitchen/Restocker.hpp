/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Restocker
*/

#pragma once

#include <atomic>
#include <cstddef>
#include <memory>

#include "plazza/core/ConditionVariable.hpp"
#include "plazza/core/Mutex.hpp"
#include "plazza/core/Thread.hpp"
#include "plazza/kitchen/Stock.hpp"

namespace plazza::kitchen {

    class Restocker {
        public:
        Restocker(Stock& stock, std::size_t periodMs);
        ~Restocker();

        Restocker(const Restocker&) = delete;
        Restocker& operator=(const Restocker&) = delete;

        void start();
        void stop();

        private:
        void loop();

        Stock& _stock;
        std::size_t _periodMs;
        std::atomic<bool> _stop{false};
        std::atomic<bool> _started{false};
        plazza::core::Mutex _mutex;
        plazza::core::ConditionVariable _cv;
        std::unique_ptr<plazza::core::Thread> _thread;
    };

}  // namespace plazza::kitchen

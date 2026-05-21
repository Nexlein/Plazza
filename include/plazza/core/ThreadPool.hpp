/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** ThreadPool
*/

#pragma once

#include <functional>
#include <utility>
#include <vector>

#include "plazza/core/SafeQueue.hpp"
#include "plazza/core/Thread.hpp"

namespace plazza::core {

    class ThreadPool {
        public:
        ThreadPool(size_t numThreads);
        ~ThreadPool();

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        template <typename Function, typename... Args>
        void enqueue(Function&& f, Args&&... args)
        {
            auto task = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
            _taskQueue.push(task);
        }

        private:
        std::vector<Thread*> _workers;
        SafeQueue<std::function<void()>> _taskQueue;

        void workerRoutine();
    };

}  // namespace plazza::core

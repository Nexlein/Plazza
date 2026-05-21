/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** SafeQueue
*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

#include "plazza/core/ISafeQueue.hpp"

namespace plazza::core {

    template <typename T>
    class SafeQueue : public ISafeQueue<T> {
        public:
        void push(T value) override
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _queue.push(value);
            }
            _condVar.notify_one();
        }

        bool tryPop(T& value) override
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_queue.empty()) {
                return false;
            }
            value = _queue.front();
            _queue.pop();
            return true;
        }

        T pop() override
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _condVar.wait(lock, [this]() { return !_queue.empty(); });

            T value = _queue.front();
            _queue.pop();

            return value;
        }

        private:
        std::queue<T> _queue;
        std::mutex _mutex;
        std::condition_variable _condVar;
    };

}  // namespace plazza::core

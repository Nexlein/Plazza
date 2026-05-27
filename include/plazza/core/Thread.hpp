/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Thread
*/

#pragma once

#include <pthread.h>

#include <functional>
#include <utility>

namespace plazza::core {

    class Thread {
        public:
        enum class Status { NOT_STARTED, RUNNING, DEAD };

        template <typename Function, typename... Args>
        Thread(Function&& f, Args&&... args) : _status(Status::NOT_STARTED)
        {
            _task = [f = std::forward<Function>(f), ... args = std::forward<Args>(args)]() {
                std::invoke(f, args...);
            };
        }

        ~Thread();

        Thread(const Thread&) = delete;
        Thread& operator=(const Thread&) = delete;

        void start();
        void join();
        Status getStatus() const;

        private:
        pthread_t _thread;
        Status _status;
        std::function<void()> _task;

        static void* wrapper(void* thisPtr);
    };

}  // namespace plazza::core

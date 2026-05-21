/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** ThreadPool
*/

#include "plazza/core/ThreadPool.hpp"

namespace plazza::core {

    ThreadPool::ThreadPool(size_t numThreads)
    {
        for (size_t i = 0; i < numThreads; ++i) {
            Thread* worker = new Thread(&ThreadPool::workerRoutine, this);
            worker->start();
            _workers.push_back(worker);
        }
    }

    ThreadPool::~ThreadPool()
    {
        for (size_t i = 0; i < _workers.size(); ++i) _taskQueue.push(nullptr);

        for (Thread* worker : _workers) {
            worker->join();
            delete worker;
        }
    }

    void ThreadPool::workerRoutine()
    {
        while (true) {
            std::function<void()> task = _taskQueue.pop();

            if (!task) break;

            task();
        }
    }

}  // namespace plazza::core

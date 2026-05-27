/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Restocker
*/

#include "plazza/kitchen/Restocker.hpp"

#include <chrono>

#include "plazza/core/ScopedLock.hpp"

namespace plazza::kitchen {

    Restocker::Restocker(Stock& stock, std::size_t periodMs) : _stock(stock), _periodMs(periodMs) {}

    Restocker::~Restocker() { stop(); }

    void Restocker::start()
    {
        if (_started.exchange(true)) return;
        _thread = std::make_unique<plazza::core::Thread>([this]() { loop(); });
        _thread->start();
    }

    void Restocker::stop()
    {
        if (_stop.exchange(true)) return;
        {
            plazza::core::ScopedLock lock(&_mutex);
        }
        _cv.notifyAll();
        if (_thread) {
            _thread->join();
            _thread.reset();
        }
    }

    void Restocker::loop()
    {
        while (!_stop.load()) {
            {
                plazza::core::ScopedLock lock(&_mutex);
                if (_stop.load()) break;
                _cv.waitFor(_mutex, std::chrono::milliseconds(_periodMs));
            }
            if (_stop.load()) break;
            _stock.restockOnce();
        }
    }

}  // namespace plazza::kitchen

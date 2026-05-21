/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** ScopedLock
*/

#include "plazza/core/ScopedLock.hpp"

namespace plazza::core {

    ScopedLock::ScopedLock(IMutex* mutex) : _mutex(mutex)
    {
        if (_mutex) _mutex->lock();
    }

    ScopedLock::~ScopedLock()
    {
        if (_mutex) _mutex->unlock();
    }

}  // namespace plazza::core

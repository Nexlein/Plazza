/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Mutex
*/

#include "plazza/core/Mutex.hpp"

namespace plazza::core {

    Mutex::Mutex() { pthread_mutex_init(&_mutex, nullptr); }

    Mutex::~Mutex() { pthread_mutex_destroy(&_mutex); }

    void Mutex::lock() { pthread_mutex_lock(&_mutex); }

    void Mutex::unlock() { pthread_mutex_unlock(&_mutex); }

    void Mutex::trylock() { pthread_mutex_trylock(&_mutex); }

    pthread_mutex_t& Mutex::getMutex() { return _mutex; }
}  // namespace plazza::core

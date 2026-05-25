/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** ConditionVariable
*/

#include "plazza/core/ConditionVariable.hpp"
#include "plazza/core/Mutex.hpp"
#include <iostream>
#include <ctime>

plazza::core::ConditionVariable::ConditionVariable() { pthread_cond_init(&_cond, nullptr); }

void plazza::core::ConditionVariable::notifyOne() { pthread_cond_signal(&_cond); }

void plazza::core::ConditionVariable::notifyAll() { pthread_cond_broadcast(&_cond); }

void plazza::core::ConditionVariable::wait(plazza::core::IMutex& mutex)
{
    auto *m = dynamic_cast<plazza::core::Mutex*>(&mutex);
    if (!m)
        throw std::invalid_argument("ConditionVariable requires a POSIX Mutex");
    pthread_cond_wait(&_cond, &m->getMutex());
}

void plazza::core::ConditionVariable::waitFor(plazza::core::IMutex& mutex, std::chrono::milliseconds duration)
{
    auto *m = dynamic_cast<plazza::core::Mutex*>(&mutex);
    if (!m)
        throw std::invalid_argument("ConditionVariable requires a POSIX Mutex");

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    auto ms = duration.count();
    ts.tv_sec  += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1'000'000;
    if (ts.tv_nsec >= 1'000'000'000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1'000'000'000L;
    }
    int ret = pthread_cond_timedwait(&_cond, &m->getMutex(), &ts);
    if (ret != 0 && ret != ETIMEDOUT)
        throw std::runtime_error("pthread_cond_timedwait failed");
}

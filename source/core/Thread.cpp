/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Thread
*/

#include "plazza/core/Thread.hpp"

namespace plazza::core {

    Thread::~Thread()
    {
        if (_status == Status::RUNNING) join();
    }

    void Thread::start()
    {
        if (_status == Status::NOT_STARTED) {
            _status = Status::RUNNING;
            pthread_create(&_thread, nullptr, Thread::wrapper, this);
        }
    }

    void Thread::join()
    {
        if (_status == Status::RUNNING || _status == Status::DEAD) {
            pthread_join(_thread, nullptr);
            _status = Status::DEAD;
        }
    }

    Thread::Status Thread::getStatus() const { return _status; }

    void* Thread::wrapper(void* thisPtr)
    {
        Thread* self = static_cast<Thread*>(thisPtr);

        if (self->_task) self->_task();

        self->_status = Status::DEAD;
        return nullptr;
    }

}  // namespace plazza::core

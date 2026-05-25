/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Process
*/

#include "plazza/core/Process.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <stdexcept>

plazza::core::Process::Process(std::function<int()> entry)
    : _pid(-1), _state(State::NOT_STARTED), _exitStatus(0)
{
    _pid = fork();
    if (_pid < 0) throw std::runtime_error("fork() failed");

    if (_pid == 0) {
        int result = entry();
        _exit(result);
    }

    _state = State::RUNNING;
}

plazza::core::Process::~Process()
{
    if (_state == State::RUNNING) wait();
}

plazza::core::Process::Process(Process&& other) noexcept
    : _pid(other._pid), _state(other._state), _exitStatus(other._exitStatus)
{
    other._pid = -1;
    other._state = State::NOT_STARTED;
}

plazza::core::Process& plazza::core::Process::operator=(Process&& other) noexcept
{
    if (this != &other) {
        if (_state == State::RUNNING) wait();
        _pid = other._pid;
        _state = other._state;
        _exitStatus = other._exitStatus;
        other._pid = -1;
        other._state = State::NOT_STARTED;
    }
    return *this;
}

int plazza::core::Process::wait()
{
    if (_state == State::EXITED) return _exitStatus;
    if (_state == State::NOT_STARTED) return 0;

    int status = 0;
    waitpid(_pid, &status, 0);
    _exitStatus = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    _state = State::EXITED;
    return _exitStatus;
}

std::optional<int> plazza::core::Process::tryWait()
{
    if (_state == State::EXITED) return _exitStatus;
    if (_state == State::NOT_STARTED) return std::nullopt;

    int status = 0;
    pid_t result = waitpid(_pid, &status, WNOHANG);
    if (result == 0) return std::nullopt;

    _exitStatus = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    _state = State::EXITED;
    return _exitStatus;
}

pid_t plazza::core::Process::getPid() const { return _pid; }

plazza::core::Process::State plazza::core::Process::getState() const { return _state; }

int plazza::core::Process::getExitStatus() const { return _exitStatus; }
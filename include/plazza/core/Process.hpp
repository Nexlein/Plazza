/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Process
*/

#pragma once

#include <sys/types.h>

#include <functional>
#include <optional>

namespace plazza::core {

    class Process {
        public:
        enum class State { NOT_STARTED, RUNNING, EXITED };

        explicit Process(std::function<int()> entry);
        ~Process();

        Process(const Process&) = delete;
        Process& operator=(const Process&) = delete;
        Process(Process&&) noexcept;
        Process& operator=(Process&&) noexcept;

        int wait();
        std::optional<int> tryWait();

        pid_t getPid() const;
        State getState() const;
        int getExitStatus() const;

        private:
        pid_t _pid;
        State _state;
        int _exitStatus;
    };

}  // namespace plazza::core

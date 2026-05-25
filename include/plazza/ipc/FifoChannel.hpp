/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** FifoChannel
*/

#pragma once

#include <string>

#include "plazza/ipc/IIpcChannel.hpp"

namespace plazza::ipc {

    class FifoChannel : public IIpcChannel {
        public:
        enum class Role { Owner, Peer };

        static void create(const std::string& basePath);

        FifoChannel(std::string basePath, Role role);
        ~FifoChannel() override;

        FifoChannel(const FifoChannel&) = delete;
        FifoChannel& operator=(const FifoChannel&) = delete;
        FifoChannel(FifoChannel&&) noexcept;
        FifoChannel& operator=(FifoChannel&&) noexcept;

        void send(const std::vector<uint8_t>& payload) override;
        std::optional<std::vector<uint8_t>> recv() override;

        private:
        std::string _pathFwd;
        std::string _pathBwd;
        int _writeFd{-1};
        int _readFd{-1};
        Role _role;
    };

}  // namespace plazza::ipc

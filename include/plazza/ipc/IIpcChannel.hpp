/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** IIpcChannel
*/

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace plazza::ipc {

    class IIpcChannel {
        public:
        virtual ~IIpcChannel() = default;

        virtual void send(const std::vector<uint8_t>& payload) = 0;

        virtual std::optional<std::vector<uint8_t>> recv() = 0;
    };

}  // namespace plazza::ipc

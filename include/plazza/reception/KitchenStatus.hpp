/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** KitchenStatus
*/

#pragma once

#include <sys/types.h>

#include <cstddef>

#include "plazza/protocol/Frame.hpp"

namespace plazza::reception {

    // A point-in-time snapshot of one kitchen, produced by Reception::pollStatus()
    // under the registry lock so the data can be handed to the graphical renderer
    // without exposing live KitchenHandle pointers across threads.
    struct KitchenStatus {
        std::size_t id{0};
        pid_t pid{-1};
        bool online{false};  // false when the status query timed out
        plazza::protocol::StatusReplyFrame reply{};
    };

}  // namespace plazza::reception

/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Frame
*/

#pragma once

#include <cstdint>
#include <variant>
#include <vector>

namespace plazza::protocol {

    enum class FrameKind : uint8_t {
        Heartbeat = 1,
        Ack = 2,
        Request = 10,
        Accepted = 11,
        Rejected = 12,
        Completed = 13,
        StatusQuery = 20,
        StatusReply = 21,
    };

    struct HeartbeatFrame {
        uint32_t token{0};
    };

    struct AckFrame {
        uint32_t token{0};
    };

    using AnyFrame = std::variant<HeartbeatFrame, AckFrame>;

    std::vector<uint8_t> encode(const HeartbeatFrame& frame);
    std::vector<uint8_t> encode(const AckFrame& frame);
    std::vector<uint8_t> encode(const AnyFrame& frame);

    AnyFrame decode(const std::vector<uint8_t>& bytes);

    FrameKind peekKind(const std::vector<uint8_t>& bytes);

}  // namespace plazza::protocol

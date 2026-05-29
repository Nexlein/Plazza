/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Frame
*/

#pragma once

#include <array>
#include <cstdint>
#include <variant>
#include <vector>

#include "plazza/pizza/PizzaSize.hpp"
#include "plazza/pizza/PizzaType.hpp"

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

    struct RequestFrame {
        uint32_t orderId{0};
        uint32_t pizzaIdx{0};
        plazza::pizza::PizzaType type{};
        plazza::pizza::PizzaSize size{};
    };

    struct AcceptedFrame {
        uint32_t orderId{0};
        uint32_t pizzaIdx{0};
    };

    struct RejectedFrame {
        uint32_t orderId{0};
        uint32_t pizzaIdx{0};
    };

    struct CompletedFrame {
        uint32_t orderId{0};
        uint32_t pizzaIdx{0};
    };

    struct StatusQueryFrame {};

    struct StatusReplyFrame {
        uint32_t cooksTotal{0};
        uint32_t outstanding{0};
        std::array<uint32_t, 9> stock{};
    };

    using AnyFrame =
        std::variant<HeartbeatFrame, AckFrame, RequestFrame, AcceptedFrame, RejectedFrame,
                     CompletedFrame, StatusQueryFrame, StatusReplyFrame>;

    std::vector<uint8_t> encode(const HeartbeatFrame& frame);
    std::vector<uint8_t> encode(const AckFrame& frame);
    std::vector<uint8_t> encode(const RequestFrame& frame);
    std::vector<uint8_t> encode(const AcceptedFrame& frame);
    std::vector<uint8_t> encode(const RejectedFrame& frame);
    std::vector<uint8_t> encode(const CompletedFrame& frame);
    std::vector<uint8_t> encode(const StatusQueryFrame& frame);
    std::vector<uint8_t> encode(const StatusReplyFrame& frame);
    std::vector<uint8_t> encode(const AnyFrame& frame);

    AnyFrame decode(const std::vector<uint8_t>& bytes);

    FrameKind peekKind(const std::vector<uint8_t>& bytes);

}  // namespace plazza::protocol

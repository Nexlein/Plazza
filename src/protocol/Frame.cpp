/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Frame
*/

#include "plazza/protocol/Frame.hpp"

#include <stdexcept>

#include "plazza/core/SerialBuffer.hpp"

namespace plazza::protocol {

    namespace {

        std::vector<uint8_t> finish(FrameKind kind, plazza::core::SerialBuffer& body)
        {
            std::vector<uint8_t> out;
            out.reserve(1 + body.bytes().size());
            out.push_back(static_cast<uint8_t>(kind));
            out.insert(out.end(), body.bytes().begin(), body.bytes().end());
            return out;
        }

    }  // namespace

    std::vector<uint8_t> encode(const HeartbeatFrame& frame)
    {
        plazza::core::SerialBuffer body;
        body << frame.token;
        return finish(FrameKind::Heartbeat, body);
    }

    std::vector<uint8_t> encode(const AckFrame& frame)
    {
        plazza::core::SerialBuffer body;
        body << frame.token;
        return finish(FrameKind::Ack, body);
    }

    std::vector<uint8_t> encode(const RequestFrame& frame)
    {
        plazza::core::SerialBuffer body;
        body << frame.orderId << frame.pizzaIdx << frame.type << frame.size;
        return finish(FrameKind::Request, body);
    }

    std::vector<uint8_t> encode(const AcceptedFrame& frame)
    {
        plazza::core::SerialBuffer body;
        body << frame.orderId << frame.pizzaIdx;
        return finish(FrameKind::Accepted, body);
    }

    std::vector<uint8_t> encode(const RejectedFrame& frame)
    {
        plazza::core::SerialBuffer body;
        body << frame.orderId << frame.pizzaIdx;
        return finish(FrameKind::Rejected, body);
    }

    std::vector<uint8_t> encode(const CompletedFrame& frame)
    {
        plazza::core::SerialBuffer body;
        body << frame.orderId << frame.pizzaIdx;
        return finish(FrameKind::Completed, body);
    }

    std::vector<uint8_t> encode(const StatusQueryFrame&)
    {
        plazza::core::SerialBuffer body;
        return finish(FrameKind::StatusQuery, body);
    }

    std::vector<uint8_t> encode(const StatusReplyFrame& frame)
    {
        plazza::core::SerialBuffer body;
        body << frame.cooksTotal << frame.outstanding;
        return finish(FrameKind::StatusReply, body);
    }

    std::vector<uint8_t> encode(const AnyFrame& frame)
    {
        return std::visit([](const auto& f) { return encode(f); }, frame);
    }

    FrameKind peekKind(const std::vector<uint8_t>& bytes)
    {
        if (bytes.empty()) throw std::runtime_error("decode: empty frame");
        return static_cast<FrameKind>(bytes[0]);
    }

    AnyFrame decode(const std::vector<uint8_t>& bytes)
    {
        if (bytes.empty()) throw std::runtime_error("decode: empty frame");
        FrameKind kind = static_cast<FrameKind>(bytes[0]);

        plazza::core::SerialBuffer body(std::vector<uint8_t>(bytes.begin() + 1, bytes.end()));

        switch (kind) {
            case FrameKind::Heartbeat: {
                HeartbeatFrame frame;
                body >> frame.token;
                return frame;
            }
            case FrameKind::Ack: {
                AckFrame frame;
                body >> frame.token;
                return frame;
            }
            case FrameKind::Request: {
                RequestFrame frame;
                body >> frame.orderId >> frame.pizzaIdx >> frame.type >> frame.size;
                return frame;
            }
            case FrameKind::Accepted: {
                AcceptedFrame frame;
                body >> frame.orderId >> frame.pizzaIdx;
                return frame;
            }
            case FrameKind::Rejected: {
                RejectedFrame frame;
                body >> frame.orderId >> frame.pizzaIdx;
                return frame;
            }
            case FrameKind::Completed: {
                CompletedFrame frame;
                body >> frame.orderId >> frame.pizzaIdx;
                return frame;
            }
            case FrameKind::StatusQuery:
                return StatusQueryFrame{};
            case FrameKind::StatusReply: {
                StatusReplyFrame frame;
                body >> frame.cooksTotal >> frame.outstanding;
                return frame;
            }
            default:
                throw std::runtime_error("decode: unsupported FrameKind");
        }
    }

}  // namespace plazza::protocol

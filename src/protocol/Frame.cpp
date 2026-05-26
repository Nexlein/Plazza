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
            default:
                throw std::runtime_error("decode: unsupported FrameKind");
        }
    }

}  // namespace plazza::protocol

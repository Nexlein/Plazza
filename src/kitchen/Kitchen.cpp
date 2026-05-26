/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Kitchen
*/

#include "plazza/kitchen/Kitchen.hpp"

#include "plazza/core/ScopedLock.hpp"

namespace plazza::kitchen {

    Kitchen::Kitchen(Config cfg) : _cfg(std::move(cfg)) {}

    int Kitchen::run()
    {
        try {
            _channel = std::make_unique<plazza::ipc::FifoChannel>(
                _cfg.channelBasePath, plazza::ipc::FifoChannel::Role::Peer);

            while (true) {
                auto raw = _channel->recv();
                if (!raw.has_value()) break;
                handleFrame(plazza::protocol::decode(*raw));
            }
            return 0;
        } catch (...) {
            return 1;
        }
    }

    void Kitchen::handleFrame(const plazza::protocol::AnyFrame& frame)
    {
        if (std::holds_alternative<plazza::protocol::HeartbeatFrame>(frame)) {
            const auto& hb = std::get<plazza::protocol::HeartbeatFrame>(frame);
            sendFrame(plazza::protocol::AckFrame{hb.token});
        }
    }

    void Kitchen::sendFrame(const plazza::protocol::AnyFrame& frame)
    {
        plazza::core::ScopedLock lock(&_sendMutex);
        _channel->send(plazza::protocol::encode(frame));
    }

}  // namespace plazza::kitchen

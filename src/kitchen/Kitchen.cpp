/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Kitchen
*/

#include "plazza/kitchen/Kitchen.hpp"

#include "plazza/core/ScopedLock.hpp"
#include "plazza/kitchen/Cook.hpp"

namespace plazza::kitchen {

    Kitchen::Kitchen(Config cfg)
        : _cfg(std::move(cfg)),
          _pool(std::make_unique<plazza::core::ThreadPool>(_cfg.cookCount))
    {
    }

    bool Kitchen::tryAccept(plazza::pizza::Pizza pizza)
    {
        {
            plazza::core::ScopedLock lock(&_capMutex);
            if (_outstanding >= 2 * _cfg.cookCount) return false;
            ++_outstanding;
        }
        _pool->enqueue([this, pizza]() {
            try {
                Cook cook(pizza, _cfg.cookingMultiplier);
                cook();
            } catch (...) {
                // Slice 5.3 will surface real cook errors via a status frame.
            }
            {
                plazza::core::ScopedLock lock(&_capMutex);
                if (_outstanding > 0) --_outstanding;
            }
            if (_onComplete) {
                try {
                    _onComplete(pizza);
                } catch (...) {
                }
            }
        });
        return true;
    }

    std::size_t Kitchen::outstanding() const
    {
        plazza::core::ScopedLock lock(&_capMutex);
        return _outstanding;
    }

    void Kitchen::setOnComplete(OnCompleteFn fn) { _onComplete = std::move(fn); }

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

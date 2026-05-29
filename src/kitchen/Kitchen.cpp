/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Kitchen
*/

#include "plazza/kitchen/Kitchen.hpp"

#include "plazza/core/ScopedLock.hpp"
#include "plazza/kitchen/Cook.hpp"
#include "plazza/pizza/Pizza.hpp"

namespace plazza::kitchen {

    Kitchen::Kitchen(Config cfg)
        : _cfg(std::move(cfg)),
          _stock(),
          _restocker(_stock, _cfg.restockMs),
          _pool(std::make_unique<plazza::core::ThreadPool>(_cfg.cookCount))
    {
        if (_cfg.restockMs > 0) _restocker.start();
    }

    Kitchen::~Kitchen()
    {
        // Wake any cook parked in Stock::waitAndConsume so _pool can join them.
        _shuttingDown.store(true);
        _stock.notifyAll();
    }

    bool Kitchen::tryAccept(plazza::pizza::Pizza pizza, uint32_t orderId, uint32_t pizzaIdx)
    {
        {
            plazza::core::ScopedLock lock(&_capMutex);
            if (_outstanding >= 2 * _cfg.cookCount) return false;
            ++_outstanding;
        }
        _pool->enqueue([this, pizza, orderId, pizzaIdx]() {
            try {
                Cook cook(pizza, _cfg.cookingMultiplier, _stock, _shuttingDown);
                cook();
            } catch (...) {
            }
            {
                plazza::core::ScopedLock lock(&_capMutex);
                if (_outstanding > 0) --_outstanding;
            }
            if (_onComplete) {
                try {
                    _onComplete(pizza, orderId, pizzaIdx);
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

            setOnComplete([this](const plazza::pizza::Pizza&, uint32_t orderId, uint32_t pizzaIdx) {
                sendFrame(plazza::protocol::CompletedFrame{orderId, pizzaIdx});
            });

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
            return;
        }

        if (std::holds_alternative<plazza::protocol::RequestFrame>(frame)) {
            const auto& req = std::get<plazza::protocol::RequestFrame>(frame);
            auto pizza = plazza::pizza::makePizza(req.type, req.size);
            if (tryAccept(pizza, req.orderId, req.pizzaIdx)) {
                sendFrame(plazza::protocol::AcceptedFrame{req.orderId, req.pizzaIdx});
            } else {
                sendFrame(plazza::protocol::RejectedFrame{req.orderId, req.pizzaIdx});
            }
            return;
        }

        if (std::holds_alternative<plazza::protocol::StatusQueryFrame>(frame)) {
            std::array<uint32_t, 9> stock{};
            for (std::size_t i = 0; i < 9; ++i)
                stock[i] = static_cast<uint32_t>(
                    _stock.available(static_cast<plazza::pizza::Ingredient>(i)));
            sendFrame(plazza::protocol::StatusReplyFrame{static_cast<uint32_t>(_cfg.cookCount),
                                                         static_cast<uint32_t>(outstanding()),
                                                         stock});
            return;
        }
    }

    void Kitchen::sendFrame(const plazza::protocol::AnyFrame& frame)
    {
        plazza::core::ScopedLock lock(&_sendMutex);
        _channel->send(plazza::protocol::encode(frame));
    }

}  // namespace plazza::kitchen

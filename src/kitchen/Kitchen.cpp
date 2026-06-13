/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Kitchen
*/

#include "plazza/kitchen/Kitchen.hpp"

#include <unistd.h>

#include <thread>

#include "plazza/core/ScopedLock.hpp"
#include "plazza/kitchen/Cook.hpp"
#include "plazza/pizza/Pizza.hpp"

namespace plazza::kitchen {

    namespace {
        long long nowMs()
        {
            using namespace std::chrono;
            return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
        }
    }  // namespace

    Kitchen::Kitchen(Config cfg)
        : _cfg(std::move(cfg)),
          _stock(),
          _restocker(_stock, _cfg.restockMs),
          _pool(std::make_unique<plazza::core::ThreadPool>(_cfg.cookCount))
    {
        _cookSlots.resize(_cfg.cookCount);
        if (_cfg.restockMs > 0) _restocker.start();
        _lastActiveTimeMs = nowMs();
    }

    Kitchen::~Kitchen()
    {
        // Wake any cook parked in Stock::waitAndConsume so _pool can join them.
        _shuttingDown.store(true);
        _stock.notifyAll();
        if (_idleThread) _idleThread->join();
    }

    bool Kitchen::tryAccept(plazza::pizza::Pizza pizza, uint32_t orderId, uint32_t pizzaIdx)
    {
        {
            plazza::core::ScopedLock lock(&_capMutex);
            if (_outstanding >= 2 * _cfg.cookCount) return false;
            ++_outstanding;
        }
        _pool->enqueue([this, pizza, orderId, pizzaIdx]() {
            // Claim a free cook slot for the renderer's per-cook view. A free
            // slot always exists because the pool runs at most cookCount tasks.
            std::size_t slot = _cookSlots.size();
            {
                plazza::core::ScopedLock lock(&_cookStateMutex);
                for (std::size_t i = 0; i < _cookSlots.size(); ++i) {
                    if (!_cookSlots[i].busy) {
                        slot = i;
                        _cookSlots[i] = CookSlotState{true, false, pizza.type, pizza.size, 0, 0};
                        break;
                    }
                }
            }
            try {
                Cook cook(pizza, _cfg.cookingMultiplier, _stock, _shuttingDown);
                cook.setOnBakeStart([this, slot](long long ms) {
                    if (slot >= _cookSlots.size()) return;
                    plazza::core::ScopedLock lock(&_cookStateMutex);
                    _cookSlots[slot].baking = true;
                    _cookSlots[slot].startMs = nowMs();
                    _cookSlots[slot].totalMs = ms;
                });
                cook();
            } catch (...) {
            }
            if (slot < _cookSlots.size()) {
                plazza::core::ScopedLock lock(&_cookStateMutex);
                _cookSlots[slot] = CookSlotState{};
            }
            {
                plazza::core::ScopedLock lock(&_capMutex);
                if (_outstanding > 0) --_outstanding;
                if (_outstanding == 0) {
                    using namespace std::chrono;
                    _lastActiveTimeMs.store(
                        duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
                            .count());
                }
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

            _idleThread = std::make_unique<plazza::core::Thread>([this]() {
                while (!_shuttingDown.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    if (_shuttingDown.load()) break;

                    long long lastActive = _lastActiveTimeMs.load();
                    long long now = nowMs();
                    if (outstanding() == 0 && (now - lastActive >= 5000)) {
                        if (_channel) _channel->closeWrite();
                        _exit(0);
                    }
                }
            });
            _idleThread->start();

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

            plazza::protocol::StatusReplyFrame reply{static_cast<uint32_t>(_cfg.cookCount),
                                                     static_cast<uint32_t>(outstanding()), stock};
            {
                plazza::core::ScopedLock lock(&_cookStateMutex);
                const long long now = nowMs();
                reply.cooks.reserve(_cookSlots.size());
                for (const auto& s : _cookSlots) {
                    plazza::protocol::CookSlot c;
                    if (!s.busy) {
                        c.state = plazza::protocol::CookState::Idle;
                    } else if (!s.baking) {
                        c.state = plazza::protocol::CookState::Waiting;
                        c.type = s.type;
                        c.size = s.size;
                    } else {
                        c.state = plazza::protocol::CookState::Baking;
                        c.type = s.type;
                        c.size = s.size;
                        long long elapsed = now - s.startMs;
                        if (elapsed < 0) elapsed = 0;
                        if (s.totalMs > 0 && elapsed > s.totalMs) elapsed = s.totalMs;
                        c.elapsedMs = static_cast<uint32_t>(elapsed);
                        c.totalMs = static_cast<uint32_t>(s.totalMs);
                    }
                    reply.cooks.push_back(c);
                }
            }
            sendFrame(reply);
            return;
        }
    }

    void Kitchen::sendFrame(const plazza::protocol::AnyFrame& frame)
    {
        plazza::core::ScopedLock lock(&_sendMutex);
        _channel->send(plazza::protocol::encode(frame));
    }

}  // namespace plazza::kitchen

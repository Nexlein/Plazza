/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Reception
*/

#include "plazza/reception/Reception.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "plazza/core/ScopedLock.hpp"
#include "plazza/pizza/Pizza.hpp"
#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Shell.hpp"
#include "plazza/render/Renderer.hpp"

namespace plazza::reception {

    Reception::Reception(float cookingMultiplier, std::size_t cooksPerKitchen,
                         std::size_t restockMs, bool graphic)
        : _cookingMultiplier(cookingMultiplier),
          _cooksPerKitchen(cooksPerKitchen),
          _restockMs(restockMs),
          _graphic(graphic),
          _dispatcherThread(nullptr),
          _reaperThread(nullptr)
    {
    }

    Reception::~Reception()
    {
        _running = false;
        _pendingCv.notifyOne();
        if (_dispatcherThread) _dispatcherThread->join();
        if (_reaperThread) _reaperThread->join();
        // _registry (declared after _logger) is destroyed first, joining the
        // kitchen reader threads while the logger they write to is still alive.
    }

    int Reception::run()
    {
        _logger = std::make_unique<Logger>(_logPath, /*mirrorStdout=*/false);
        std::ostringstream banner;
        banner << "reception ready (multiplier=" << _cookingMultiplier
               << ", cooks=" << _cooksPerKitchen << ", restock_ms=" << _restockMs << ")";
        _logger->info(banner.str());

        auto onOrder = [this](const Order& order) {
            {
                core::ScopedLock lock(&_pendingMutex);
                _pending.push(order);
            }
            _pendingCv.notifyOne();
        };

        dispatchOrders();

        _reaperThread = std::make_unique<core::Thread>([this]() {
            while (_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                if (!_running) break;

                core::ScopedLock lock(&_registryMutex);
                for (auto it = _registry.begin(); it != _registry.end();) {
                    auto exitStatus = it->handle->tryWaitProcess();
                    if (exitStatus.has_value()) {
                        it = _registry.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        });
        _reaperThread->start();

        int code = 0;
        if (_graphic) {
            render::Renderer renderer(*_logger, onOrder, [this]() { return pollStatus(); });
            {
                core::ScopedLock lock(&_ioMutex);
                _uiEventSink = [&renderer](const std::string& line) { renderer.pushEvent(line); };
            }
            code = renderer.run();
            // Detach the sink before the renderer is destroyed; holding _ioMutex
            // here waits for any in-flight handleCompleted() sink call to finish.
            {
                core::ScopedLock lock(&_ioMutex);
                _uiEventSink = nullptr;
            }
        } else {
            Shell shell(*_logger, std::cin, std::cout, onOrder, [this]() {
                core::ScopedLock lock(&_registryMutex);
                std::vector<KitchenHandle*> handles;
                for (const auto& kr : _registry) handles.push_back(kr.handle.get());
                return handles;
            });
            code = shell.run();
        }
        _logger->info("reception shutting down");
        return code;
    }

    std::vector<KitchenStatus> Reception::pollStatus()
    {
        core::ScopedLock lock(&_registryMutex);
        for (auto& kr : _registry) {
            try {
                kr.handle->triggerStatusQuery();
            } catch (...) {
                // Kitchen died before we could query it; reported as offline below.
            }
        }

        std::vector<KitchenStatus> out;
        out.reserve(_registry.size());
        const auto start = std::chrono::steady_clock::now();
        const auto budget = std::chrono::milliseconds(120);
        for (auto& kr : _registry) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
            auto timeout = budget - elapsed;
            if (timeout.count() < 0) timeout = std::chrono::milliseconds(0);

            KitchenStatus s;
            s.id = kr.id;
            s.pid = kr.handle->pid();
            auto reply = kr.handle->collectStatusReply(timeout);
            if (reply.has_value()) {
                s.online = true;
                s.reply = *reply;
            }
            out.push_back(std::move(s));
        }
        return out;
    }

    void Reception::spawnKitchen()
    {
        std::size_t id = _registry.size();
        kitchen::Kitchen::Config cfg = {
            .cookCount = _cooksPerKitchen,
            .cookingMultiplier = _cookingMultiplier,
            .restockMs = _restockMs,
            .channelBasePath = "/tmp/plazza_kitchen_" + std::to_string(id)};
        _registry.push_back({id, 0, std::make_unique<KitchenHandle>(cfg, id)});
        _registry.back().handle->onFrame([this, id](const protocol::AnyFrame& frame) {
            if (std::holds_alternative<protocol::CompletedFrame>(frame)) {
                const auto& done = std::get<protocol::CompletedFrame>(frame);
                releaseSlot(id);
                handleCompleted(done.orderId);
            } else if (std::holds_alternative<protocol::RejectedFrame>(frame)) {
                const auto& rejected = std::get<protocol::RejectedFrame>(frame);
                releaseSlot(id);
                std::ostringstream msg;
                msg << "kitchen " << id << " rejected order #" << rejected.orderId;
                _logger->warn(msg.str());
            }
        });
    }

    void Reception::releaseSlot(std::size_t kitchenId)
    {
        core::ScopedLock lock(&_registryMutex);
        auto it =
            std::find_if(_registry.begin(), _registry.end(),
                         [kitchenId](const KitchenRegistry& kr) { return kr.id == kitchenId; });
        if (it != _registry.end() && it->occupiedSlots > 0) --it->occupiedSlots;
    }

    void Reception::handleCompleted(uint32_t orderId)
    {
        const OrderTracker::Update update = _orderTracker.complete(orderId);
        if (!update.known) return;

        core::ScopedLock lock(&_ioMutex);
        _logger->info(update.pizzaLine);
        if (update.orderComplete) _logger->info(update.orderLine);

        if (_uiEventSink) {
            // Graphic mode: feed the event log instead of writing to the canvas.
            _uiEventSink(update.pizzaLine);
            if (update.orderComplete) _uiEventSink(update.orderLine);
        } else {
            std::cout << update.pizzaLine << '\n';
            if (update.orderComplete) std::cout << update.orderLine << '\n';
            std::cout.flush();
        }
    }

    void Reception::dispatchOrders()
    {
        _dispatcherThread = std::make_unique<core::Thread>([this]() {
            while (_running) {
                Order order;
                {
                    core::ScopedLock lock(&_pendingMutex);
                    while (_pending.empty() && _running) _pendingCv.wait(_pendingMutex);
                    if (!_running) return;
                    order = _pending.front();
                    _pending.pop();
                }

                const uint32_t orderId = _orderTracker.add(order.count, order.type, order.size);

                for (std::size_t i = 0; i < order.count; i++) {
                    pizza::Pizza pizza = pizza::makePizza(order.type, order.size);
                    core::ScopedLock lock(&_registryMutex);

                    KitchenRegistry* best = nullptr;
                    for (KitchenRegistry& kr : _registry) {
                        if (!best || kr.occupiedSlots < best->occupiedSlots ||
                            (kr.occupiedSlots == best->occupiedSlots && kr.id < best->id))
                            best = &kr;
                    }

                    if (!best || best->occupiedSlots >= _cooksPerKitchen * 2) {
                        spawnKitchen();
                        best = &_registry.back();
                    }
                    best->handle->sendFrame(
                        protocol::RequestFrame{.orderId = orderId,
                                               .pizzaIdx = static_cast<uint32_t>(i),
                                               .type = pizza.type,
                                               .size = pizza.size});
                    best->occupiedSlots++;
                }
            }
        });
        _dispatcherThread->start();
    }

}  // namespace plazza::reception

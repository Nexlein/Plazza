/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Reception
*/

#include "plazza/reception/Reception.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "plazza/core/ScopedLock.hpp"
#include "plazza/pizza/Pizza.hpp"
#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Shell.hpp"

namespace plazza::reception {

    Reception::Reception(float cookingMultiplier, std::size_t cooksPerKitchen,
                         std::size_t restockMs)
        : _cookingMultiplier(cookingMultiplier),
          _cooksPerKitchen(cooksPerKitchen),
          _restockMs(restockMs),
          _dispatcherThread(nullptr)
    {
    }

    Reception::~Reception()
    {
        _running = false;
        _pendingCv.notifyOne();
        if (_dispatcherThread) _dispatcherThread->join();
        // Destroy kitchen handles (joining their reader threads) while _logger
        // is still alive, since those threads log pizza completions.
        _registry.clear();
    }

    int Reception::run()
    {
        _logger = std::make_unique<Logger>(_logPath, /*mirrorStdout=*/false);
        std::ostringstream banner;
        banner << "reception ready (multiplier=" << _cookingMultiplier
               << ", cooks=" << _cooksPerKitchen << ", restock_ms=" << _restockMs << ")";
        _logger->info(banner.str());

        Shell shell(*_logger, std::cin, std::cout, [this](const Order& order) {
            {
                core::ScopedLock lock(&_pendingMutex);
                _pending.push(order);
            }
            _pendingCv.notifyOne();
        });

        dispatchOrders();
        const int code = shell.run();
        _logger->info("reception shutting down");
        return code;
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
        std::cout << update.pizzaLine << '\n';
        _logger->info(update.pizzaLine);
        if (update.orderComplete) {
            std::cout << update.orderLine << '\n';
            _logger->info(update.orderLine);
        }
        std::cout.flush();
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

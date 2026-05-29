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

namespace plazza::reception {

    Reception::Reception(float cookingMultiplier, std::size_t cooksPerKitchen,
                         std::size_t restockMs)
        : _cookingMultiplier(cookingMultiplier),
          _cooksPerKitchen(cooksPerKitchen),
          _restockMs(restockMs),
          _dispatcherThread(nullptr),
          _reaperThread(nullptr),
          _orderId(0)
    {
    }

    Reception::~Reception()
    {
        _running = false;
        _pendingCv.notifyOne();
        if (_dispatcherThread) _dispatcherThread->join();
        if (_reaperThread) _reaperThread->join();
    }

    int Reception::run()
    {
        Logger logger(_logPath, /*mirrorStdout=*/false);
        std::ostringstream banner;
        banner << "reception ready (multiplier=" << _cookingMultiplier
               << ", cooks=" << _cooksPerKitchen << ", restock_ms=" << _restockMs << ")";
        logger.info(banner.str());

        Shell shell(
            logger, std::cin, std::cout,
            [this](const Order& order) {
                {
                    core::ScopedLock lock(&_pendingMutex);
                    _pending.push(order);
                }
                _pendingCv.notifyOne();
            },
            [this]() {
                core::ScopedLock lock(&_registryMutex);
                std::vector<KitchenHandle*> handles;
                for (const auto& kr : _registry) handles.push_back(kr.handle.get());
                return handles;
            });

        dispatchOrders(logger);

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

        const int code = shell.run();
        logger.info("reception shutting down");
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
            core::ScopedLock lock(&_registryMutex);
            auto it = std::find_if(_registry.begin(), _registry.end(),
                                   [id](const KitchenRegistry& kr) { return kr.id == id; });
            if (it == _registry.end()) return;

            if (std::holds_alternative<protocol::CompletedFrame>(frame))
                it->occupiedSlots--;
            else if (std::holds_alternative<protocol::RejectedFrame>(frame))
                it->occupiedSlots--;
        });
    }

    // update les occupations des kitchens dans le registry
    void Reception::dispatchOrders(Logger& logger)
    {
        _dispatcherThread = std::make_unique<core::Thread>([this, &logger]() {
            while (_running) {
                Order order;
                {
                    core::ScopedLock lock(&_pendingMutex);
                    while (_running && _pending.empty()) _pendingCv.wait(_pendingMutex);
                    if (!_running) break;
                    order = _pending.front();
                    _pending.pop();
                }

                for (int i = 0; i < order.count; i++) {
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
                        protocol::RequestFrame{.orderId = ++_orderId,
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

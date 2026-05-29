/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Reception
*/

#pragma once

#include <cstddef>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "plazza/core/ConditionVariable.hpp"
#include "plazza/core/Mutex.hpp"
#include "plazza/core/Thread.hpp"
#include "plazza/reception/KitchenHandle.hpp"
#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Order.hpp"

namespace plazza::reception {

    class Reception {
        public:
        Reception(float cookingMultiplier, std::size_t cooksPerKitchen, std::size_t restockMs);
        ~Reception();

        int run();
        void dispatchOrders(Logger &logger);

        struct KitchenRegistry {
            std::size_t id;
            std::size_t occupiedSlots;
            std::unique_ptr<KitchenHandle> handle;
        };

        private:
        void spawnKitchen();
        std::atomic<bool> _running{true};
        float _cookingMultiplier;
        std::size_t _cooksPerKitchen;
        std::size_t _restockMs;
        std::string _logPath = "plazza.log";
        std::unique_ptr<core::Thread> _dispatcherThread;
        std::unique_ptr<core::Thread> _reaperThread;
        std::queue<Order> _pending;
        core::Mutex _pendingMutex;
        std::vector<KitchenRegistry> _registry;
        core::Mutex _registryMutex;
        core::ConditionVariable _pendingCv;
        uint32_t _orderId;
    };

}  // namespace plazza::reception

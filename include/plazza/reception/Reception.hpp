/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Reception
*/

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "plazza/core/ConditionVariable.hpp"
#include "plazza/core/Mutex.hpp"
#include "plazza/core/Thread.hpp"
#include "plazza/reception/KitchenHandle.hpp"
#include "plazza/reception/KitchenStatus.hpp"
#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Order.hpp"
#include "plazza/reception/OrderTracker.hpp"

namespace plazza::reception {

    class Reception {
        public:
        Reception(float cookingMultiplier, std::size_t cooksPerKitchen, std::size_t restockMs,
                  bool graphic = false);
        ~Reception();

        int run();
        void dispatchOrders();

        // Queries every registered kitchen and returns a snapshot. Runs entirely
        // under _registryMutex so the reaper cannot destroy a handle mid-query;
        // used by the graphical renderer's poller thread.
        std::vector<KitchenStatus> pollStatus();

        struct KitchenRegistry {
            std::size_t id;
            std::size_t occupiedSlots;
            std::unique_ptr<KitchenHandle> handle;
        };

        private:
        void spawnKitchen();
        // Frees one busy slot of the given kitchen; called when it reports a
        // pizza completed or rejected. Takes _registryMutex internally.
        void releaseSlot(std::size_t kitchenId);
        // Reports a finished pizza to the user and the log, plus an order-level
        // summary once the last pizza of the order is done.
        void handleCompleted(uint32_t orderId);

        std::atomic<bool> _running{true};
        float _cookingMultiplier;
        std::size_t _cooksPerKitchen;
        std::size_t _restockMs;
        bool _graphic{false};
        // When set (graphic mode), user-facing completion lines are routed here
        // instead of std::cout so they don't corrupt the libcaca canvas. Guarded
        // by _ioMutex, which already serialises user-facing output.
        std::function<void(const std::string&)> _uiEventSink;
        std::string _logPath = "plazza.log";
        // _logger is declared before _registry so it outlives the kitchen
        // reader threads (which log completions) during teardown.
        std::unique_ptr<Logger> _logger;
        OrderTracker _orderTracker;
        core::Mutex _ioMutex;  // serialises user-facing stdout writes
        std::unique_ptr<core::Thread> _dispatcherThread;
        std::unique_ptr<core::Thread> _reaperThread;
        std::queue<Order> _pending;
        core::Mutex _pendingMutex;
        std::vector<KitchenRegistry> _registry;
        core::Mutex _registryMutex;
        core::ConditionVariable _pendingCv;
    };

}  // namespace plazza::reception

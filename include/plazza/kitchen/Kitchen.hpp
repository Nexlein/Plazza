/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Kitchen
*/

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "plazza/core/Mutex.hpp"
#include "plazza/core/ThreadPool.hpp"
#include "plazza/ipc/FifoChannel.hpp"
#include "plazza/kitchen/Restocker.hpp"
#include "plazza/kitchen/Stock.hpp"
#include "plazza/pizza/Pizza.hpp"
#include "plazza/protocol/Frame.hpp"

namespace plazza::kitchen {

    class Kitchen {
        public:
        struct Config {
            std::size_t cookCount;
            float cookingMultiplier;
            std::size_t restockMs;
            std::string channelBasePath;
        };

        using OnCompleteFn =
            std::function<void(const plazza::pizza::Pizza&, uint32_t orderId, uint32_t pizzaIdx)>;

        explicit Kitchen(Config cfg);
        ~Kitchen();

        Kitchen(const Kitchen&) = delete;
        Kitchen& operator=(const Kitchen&) = delete;

        int run();

        bool tryAccept(plazza::pizza::Pizza pizza, uint32_t orderId = 0, uint32_t pizzaIdx = 0);
        std::size_t outstanding() const;
        std::size_t cookCount() const { return _cfg.cookCount; }

        void setOnComplete(OnCompleteFn fn);

        private:
        void handleFrame(const plazza::protocol::AnyFrame& frame);
        void sendFrame(const plazza::protocol::AnyFrame& frame);

        // Live per-cook state, mirrored to the renderer via StatusReplyFrame.
        // A slot is claimed when a pool task starts and freed when it finishes;
        // at most _cfg.cookCount tasks run at once, so a free slot always exists.
        struct CookSlotState {
            bool busy{false};
            bool baking{false};  // false => still waiting on ingredients
            plazza::pizza::PizzaType type{};
            plazza::pizza::PizzaSize size{};
            long long startMs{0};
            long long totalMs{0};
        };

        Config _cfg;
        OnCompleteFn _onComplete;
        std::unique_ptr<plazza::ipc::FifoChannel> _channel;
        mutable plazza::core::Mutex _capMutex;
        std::size_t _outstanding{0};
        mutable plazza::core::Mutex _cookStateMutex;
        std::vector<CookSlotState> _cookSlots;
        plazza::core::Mutex _sendMutex;
        // _shuttingDown is declared before _stock so its lifetime fully covers
        // every cook's reference to it during pool teardown.
        std::atomic<bool> _shuttingDown{false};
        // Member order is load-bearing for shutdown: _pool destroys first
        // (joining cook workers), then _restocker stops the timer thread,
        // then _stock is torn down.
        Stock _stock;
        Restocker _restocker;
        std::unique_ptr<plazza::core::ThreadPool> _pool;
        std::atomic<long long> _lastActiveTimeMs{0};
        std::unique_ptr<plazza::core::Thread> _idleThread;
    };

}  // namespace plazza::kitchen

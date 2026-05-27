/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Kitchen
*/

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

#include "plazza/core/Mutex.hpp"
#include "plazza/core/ThreadPool.hpp"
#include "plazza/ipc/FifoChannel.hpp"
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
        ~Kitchen() = default;

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

        Config _cfg;
        OnCompleteFn _onComplete;
        std::unique_ptr<plazza::ipc::FifoChannel> _channel;
        mutable plazza::core::Mutex _capMutex;
        std::size_t _outstanding{0};
        plazza::core::Mutex _sendMutex;
        // _pool last so it is destroyed first: workers join before the mutexes
        // and callbacks they may touch go away.
        std::unique_ptr<plazza::core::ThreadPool> _pool;
    };

}  // namespace plazza::kitchen

/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Kitchen
*/

#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include "plazza/core/Mutex.hpp"
#include "plazza/ipc/FifoChannel.hpp"
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

        explicit Kitchen(Config cfg);
        ~Kitchen() = default;

        Kitchen(const Kitchen&) = delete;
        Kitchen& operator=(const Kitchen&) = delete;

        int run();

        private:
        void handleFrame(const plazza::protocol::AnyFrame& frame);
        void sendFrame(const plazza::protocol::AnyFrame& frame);

        Config _cfg;
        std::unique_ptr<plazza::ipc::FifoChannel> _channel;
        plazza::core::Mutex _sendMutex;
    };

}  // namespace plazza::kitchen

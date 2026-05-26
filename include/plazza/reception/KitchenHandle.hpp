/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** KitchenHandle
*/

#pragma once

#include <sys/types.h>

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>

#include "plazza/core/Mutex.hpp"
#include "plazza/core/Thread.hpp"
#include "plazza/ipc/FifoChannel.hpp"
#include "plazza/kitchen/Kitchen.hpp"
#include "plazza/protocol/Frame.hpp"

namespace plazza::core {
    class Process;
}

namespace plazza::reception {

    class KitchenHandle {
        public:
        using FrameCallback = std::function<void(const plazza::protocol::AnyFrame&)>;

        KitchenHandle(plazza::kitchen::Kitchen::Config cfg, std::size_t kitchenId);
        ~KitchenHandle();

        KitchenHandle(const KitchenHandle&) = delete;
        KitchenHandle& operator=(const KitchenHandle&) = delete;

        void onFrame(FrameCallback cb);
        void sendFrame(const plazza::protocol::AnyFrame& frame);
        void shutdown();

        std::size_t id() const { return _id; }
        pid_t pid() const;

        private:
        void readerLoop();

        std::size_t _id;
        std::unique_ptr<plazza::core::Process> _process;
        std::unique_ptr<plazza::ipc::FifoChannel> _channel;
        plazza::core::Mutex _sendMutex;
        std::unique_ptr<plazza::core::Thread> _readerThread;
        FrameCallback _onFrame;
        std::atomic<bool> _stopped{false};
    };

}  // namespace plazza::reception

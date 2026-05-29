/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** KitchenHandle
*/

#include "plazza/reception/KitchenHandle.hpp"

#include "plazza/core/Process.hpp"
#include "plazza/core/ScopedLock.hpp"

namespace plazza::reception {

    KitchenHandle::KitchenHandle(plazza::kitchen::Kitchen::Config cfg, std::size_t kitchenId)
        : _id(kitchenId)
    {
        plazza::ipc::FifoChannel::create(cfg.channelBasePath);

        _process = std::make_unique<plazza::core::Process>([cfg]() {
            plazza::kitchen::Kitchen kitchen(cfg);
            return kitchen.run();
        });

        _channel = std::make_unique<plazza::ipc::FifoChannel>(
            cfg.channelBasePath, plazza::ipc::FifoChannel::Role::Owner);

        _readerThread = std::make_unique<plazza::core::Thread>([this] { readerLoop(); });
        _readerThread->start();
    }

    KitchenHandle::~KitchenHandle() { shutdown(); }

    void KitchenHandle::onFrame(FrameCallback cb) { _onFrame = std::move(cb); }

    void KitchenHandle::sendFrame(const plazza::protocol::AnyFrame& frame)
    {
        plazza::core::ScopedLock lock(&_sendMutex);
        _channel->send(plazza::protocol::encode(frame));
    }

    void KitchenHandle::shutdown()
    {
        if (_stopped.exchange(true)) return;

        // Half-close our write end: child's recv() returns EOF, child exits run()
        // and destroys its FifoChannel, which closes its write end. Our reader
        // thread then sees EOF naturally on the back-channel and returns.
        if (_channel) _channel->closeWrite();

        if (_readerThread) {
            _readerThread->join();
            _readerThread.reset();
        }

        if (_process) {
            _process->wait();
            _process.reset();
        }

        // Now safe to destroy the channel (unlinks the FIFO files).
        _channel.reset();
    }

    void KitchenHandle::triggerStatusQuery()
    {
        plazza::core::ScopedLock lock(&_statusMutex);
        _lastStatusReply = std::nullopt;
        _waitingForStatus = true;
        sendFrame(plazza::protocol::StatusQueryFrame{});
    }

    std::optional<plazza::protocol::StatusReplyFrame> KitchenHandle::collectStatusReply(
        std::chrono::milliseconds timeout)
    {
        plazza::core::ScopedLock lock(&_statusMutex);
        auto start = std::chrono::steady_clock::now();
        while (_waitingForStatus && !_stopped.load()) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
            if (elapsed >= timeout) break;
            _statusCv.waitFor(_statusMutex, timeout - elapsed);
        }
        _waitingForStatus = false;
        return _lastStatusReply;
    }

    pid_t KitchenHandle::pid() const { return _process ? _process->getPid() : -1; }

    void KitchenHandle::readerLoop()
    {
        while (true) {
            std::optional<std::vector<uint8_t>> raw;
            try {
                raw = _channel->recv();
            } catch (...) {
                break;
            }
            if (!raw.has_value()) break;

            try {
                auto frame = plazza::protocol::decode(*raw);
                if (std::holds_alternative<plazza::protocol::StatusReplyFrame>(frame)) {
                    plazza::core::ScopedLock lock(&_statusMutex);
                    if (_waitingForStatus) {
                        _lastStatusReply = std::get<plazza::protocol::StatusReplyFrame>(frame);
                        _waitingForStatus = false;
                        _statusCv.notifyOne();
                    }
                }
                if (_onFrame) _onFrame(frame);
            } catch (...) {
                // malformed frame; drop and continue
            }
        }
    }

}  // namespace plazza::reception

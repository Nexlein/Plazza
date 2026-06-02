/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Renderer (libcaca graphical bonus)
*/

#pragma once

#include <atomic>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "plazza/core/Mutex.hpp"
#include "plazza/core/Thread.hpp"
#include "plazza/reception/KitchenStatus.hpp"
#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Order.hpp"

// Opaque libcaca handles, forward declared to keep <caca.h> out of this header.
typedef struct caca_canvas caca_canvas_t;
typedef struct caca_display caca_display_t;

namespace plazza::render {

    // Full-screen libcaca dashboard: shows each kitchen, what every cook is
    // baking with a live progress bar, ingredient stock and a scrolling event
    // log, and lets the user type pizza orders in-canvas. Replaces the text
    // Shell when the reception is launched with -g/--graphic.
    //
    // Threading contract: every caca_* call happens on the thread that calls
    // run() (the main thread). A background poller thread only mutates our own
    // snapshot/event buffers under their mutexes and never touches libcaca.
    class Renderer {
        public:
        using OnOrder = std::function<void(const reception::Order&)>;
        using PollStatus = std::function<std::vector<reception::KitchenStatus>()>;

        Renderer(reception::Logger& logger, OnOrder onOrder, PollStatus pollStatus);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        // Append a line to the scrolling event log. Thread-safe; wired to the
        // reception so pizza/order completions appear in the UI.
        void pushEvent(const std::string& line);

        // Runs the render + input loop until the user quits (ESC / window close).
        // Returns the process exit code (0 on clean quit, 84 if libcaca could
        // not initialise a display).
        int run();

        private:
        // Local view of a cook, with the absolute time the sample was taken so
        // the progress bar can be interpolated smoothly between polls.
        struct CookView {
            protocol::CookState state{protocol::CookState::Idle};
            pizza::PizzaType type{};
            pizza::PizzaSize size{};
            uint32_t elapsedMs{0};
            uint32_t totalMs{0};
        };
        struct KitchenView {
            std::size_t id{0};
            pid_t pid{-1};
            bool online{false};
            uint32_t cooksTotal{0};
            uint32_t outstanding{0};
            std::array<uint32_t, 9> stock{};
            std::vector<CookView> cooks;
            long long polledAtMs{0};
        };

        void pollLoop();
        void buildSnapshot();
        void handleInput();
        void submitOrder();

        void draw();
        void drawHeader(int width);
        int drawKitchen(const KitchenView& k, int x, int y, int width, long long nowMs);
        void drawFooter(int width, int height);

        reception::Logger& _logger;
        OnOrder _onOrder;
        PollStatus _pollStatus;

        caca_canvas_t* _canvas{nullptr};
        caca_display_t* _display{nullptr};

        std::atomic<bool> _running{true};
        std::unique_ptr<core::Thread> _pollThread;

        core::Mutex _snapMutex;
        std::vector<KitchenView> _snapshot;

        core::Mutex _eventMutex;
        std::deque<std::string> _events;

        std::string _input;  // touched only on the run() thread
    };

}  // namespace plazza::render

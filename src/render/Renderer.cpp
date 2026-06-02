/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Renderer (libcaca graphical bonus)
*/

#include "plazza/render/Renderer.hpp"

#include <caca.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>

#include "plazza/core/ScopedLock.hpp"
#include "plazza/pizza/Pizza.hpp"
#include "plazza/reception/Parser.hpp"

namespace plazza::render {

    namespace {

        long long nowMs()
        {
            using namespace std::chrono;
            return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
        }

        std::string trim(const std::string& s)
        {
            std::size_t b = s.find_first_not_of(" \t\r\n");
            if (b == std::string::npos) return "";
            std::size_t e = s.find_last_not_of(" \t\r\n");
            return s.substr(b, e - b + 1);
        }

        // One-letter labels for the nine ingredients, in Ingredient enum order.
        const char* kStockLabels[9] = {"Do", "To", "Gr", "Ha", "Mu", "St", "Eg", "Gc", "Cl"};

        constexpr int kPanelW = 34;
        constexpr int kFooterRows = 5;  // separator + 3 event lines + input line
        constexpr int kMaxCookRows = 8;

    }  // namespace

    Renderer::Renderer(reception::Logger& logger, OnOrder onOrder, PollStatus pollStatus)
        : _logger(logger), _onOrder(std::move(onOrder)), _pollStatus(std::move(pollStatus))
    {
    }

    Renderer::~Renderer()
    {
        _running.store(false);
        if (_pollThread) _pollThread->join();
        if (_display) caca_free_display(_display);
        if (_canvas) caca_free_canvas(_canvas);
    }

    void Renderer::pushEvent(const std::string& line)
    {
        core::ScopedLock lock(&_eventMutex);
        _events.push_back(line);
        while (_events.size() > 64) _events.pop_front();
    }

    int Renderer::run()
    {
        _canvas = caca_create_canvas(0, 0);
        if (!_canvas) return 84;
        _display = caca_create_display(_canvas);
        if (!_display) {
            caca_free_canvas(_canvas);
            _canvas = nullptr;
            return 84;
        }
        caca_set_display_title(_display, "Plazza");
        pushEvent("graphical mode ready - type an order, Enter to send, ESC to quit");
        _logger.info("graphic renderer started");

        _pollThread = std::make_unique<core::Thread>([this]() { pollLoop(); });
        _pollThread->start();

        while (_running.load()) {
            handleInput();
            if (!_running.load()) break;
            draw();
        }

        _logger.info("graphic renderer stopped");
        return 0;
    }

    void Renderer::pollLoop()
    {
        while (_running.load()) {
            buildSnapshot();
            // ~250ms between polls, in 10ms slices so we stop promptly on quit.
            for (int i = 0; i < 25 && _running.load(); ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void Renderer::buildSnapshot()
    {
        std::vector<reception::KitchenStatus> statuses;
        try {
            statuses = _pollStatus();
        } catch (...) {
            return;
        }

        std::vector<KitchenView> views;
        views.reserve(statuses.size());
        const long long sampledAt = nowMs();
        for (const auto& s : statuses) {
            KitchenView v;
            v.id = s.id;
            v.pid = s.pid;
            v.online = s.online;
            v.polledAtMs = sampledAt;
            if (s.online) {
                v.cooksTotal = s.reply.cooksTotal;
                v.outstanding = s.reply.outstanding;
                v.stock = s.reply.stock;
                for (const auto& c : s.reply.cooks)
                    v.cooks.push_back({c.state, c.type, c.size, c.elapsedMs, c.totalMs});
            }
            views.push_back(std::move(v));
        }

        core::ScopedLock lock(&_snapMutex);
        _snapshot = std::move(views);
    }

    void Renderer::handleInput()
    {
        caca_event_t ev;
        // Block briefly for the first event (paces the loop to ~30 fps when idle),
        // then drain anything else that is already queued.
        int got = caca_get_event(_display, CACA_EVENT_KEY_PRESS | CACA_EVENT_QUIT, &ev, 33000);
        while (got) {
            if (caca_get_event_type(&ev) & CACA_EVENT_QUIT) {
                _running.store(false);
                return;
            }
            if (caca_get_event_type(&ev) & CACA_EVENT_KEY_PRESS) {
                int ch = caca_get_event_key_ch(&ev);
                if (ch == CACA_KEY_ESCAPE) {
                    _running.store(false);
                    return;
                }
                if (ch == CACA_KEY_RETURN) {
                    submitOrder();
                } else if (ch == CACA_KEY_BACKSPACE || ch == CACA_KEY_DELETE) {
                    if (!_input.empty()) _input.pop_back();
                } else if (ch >= 0x20 && ch < 0x7f) {
                    if (_input.size() < 200) _input.push_back(static_cast<char>(ch));
                }
            }
            got = caca_get_event(_display, CACA_EVENT_KEY_PRESS | CACA_EVENT_QUIT, &ev, 0);
        }
    }

    void Renderer::submitOrder()
    {
        const std::string line = trim(_input);
        _input.clear();
        if (line.empty()) return;
        if (line == "exit" || line == "quit") {
            _running.store(false);
            return;
        }

        const auto parsed = reception::Parser::parse(line);
        if (!parsed) {
            pushEvent("rejected: malformed order '" + line + "'");
            _logger.warn("graphic: rejected order: " + line);
            return;
        }
        std::size_t total = 0;
        for (const auto& order : *parsed) {
            total += order.count;
            _onOrder(order);
        }
        pushEvent("accepted " + std::to_string(total) + " pizza(s): " + line);
        _logger.info("graphic: accepted order: " + line);
    }

    void Renderer::draw()
    {
        const int width = caca_get_canvas_width(_canvas);
        const int height = caca_get_canvas_height(_canvas);

        caca_set_color_ansi(_canvas, CACA_LIGHTGRAY, CACA_BLACK);
        caca_clear_canvas(_canvas);

        if (width < 44 || height < 14) {
            caca_set_color_ansi(_canvas, CACA_LIGHTRED, CACA_BLACK);
            caca_put_str(_canvas, 0, 0, "Terminal too small - please enlarge the window.");
            caca_refresh_display(_display);
            return;
        }

        drawHeader(width);

        std::vector<KitchenView> snap;
        {
            core::ScopedLock lock(&_snapMutex);
            snap = _snapshot;
        }

        const int regionTop = 2;
        const int regionBottom = height - kFooterRows - 1;  // last usable kitchen row
        const long long now = nowMs();

        if (snap.empty()) {
            caca_set_color_ansi(_canvas, CACA_YELLOW, CACA_BLACK);
            caca_put_str(_canvas, 2, regionTop + 1,
                         "(no kitchens running yet - submit an order below)");
        } else {
            int cookRows = 1;
            for (const auto& k : snap)
                cookRows = std::max(cookRows, static_cast<int>(k.cooksTotal));
            cookRows = std::min(cookRows, kMaxCookRows);
            const int panelH = cookRows + 5;

            const int cols = std::max(1, width / (kPanelW + 1));
            int drawn = 0;
            for (std::size_t i = 0; i < snap.size(); ++i) {
                const int col = drawn % cols;
                const int row = drawn / cols;
                const int x = col * (kPanelW + 1);
                const int y = regionTop + row * (panelH + 1);
                if (y + panelH - 1 > regionBottom) {
                    caca_set_color_ansi(_canvas, CACA_DARKGRAY, CACA_BLACK);
                    caca_printf(_canvas, 2, regionBottom,
                                "(+%zu more kitchen(s) - enlarge window to see all)",
                                snap.size() - i);
                    break;
                }
                drawKitchen(snap[i], x, y, kPanelW, now);
                ++drawn;
            }
        }

        drawFooter(width, height);
        caca_refresh_display(_display);
    }

    void Renderer::drawHeader(int width)
    {
        caca_set_color_ansi(_canvas, CACA_WHITE, CACA_BLUE);
        for (int x = 0; x < width; ++x) caca_put_char(_canvas, x, 0, ' ');
        caca_put_str(_canvas, 1, 0, "PLAZZA");
        caca_put_str(_canvas, 9, 0, "live pizzeria");
        caca_set_color_ansi(_canvas, CACA_LIGHTGRAY, CACA_BLUE);
        const char* help = "ESC quit | type order + Enter";
        int hx = width - static_cast<int>(std::string(help).size()) - 1;
        if (hx > 24) caca_put_str(_canvas, hx, 0, help);
    }

    int Renderer::drawKitchen(const KitchenView& k, int x, int y, int width, long long nowMsVal)
    {
        const int cookRows = std::min(static_cast<int>(k.cooksTotal), kMaxCookRows);
        const int h = std::max(cookRows, 1) + 5;

        caca_set_color_ansi(_canvas, k.online ? CACA_CYAN : CACA_DARKGRAY, CACA_BLACK);
        caca_draw_thin_box(_canvas, x, y, width, h);

        // Title + aggregate line.
        unsigned busy = 0;
        for (const auto& c : k.cooks)
            if (c.state != protocol::CookState::Idle) ++busy;
        const unsigned queue = k.outstanding > k.cooksTotal ? k.outstanding - k.cooksTotal : 0;

        caca_set_color_ansi(_canvas, CACA_WHITE, CACA_BLACK);
        caca_printf(_canvas, x + 1, y + 1, "Kitchen #%zu  PID %d%s", k.id,
                    static_cast<int>(k.pid), k.online ? "" : "  [offline]");
        caca_set_color_ansi(_canvas, CACA_LIGHTGRAY, CACA_BLACK);
        caca_printf(_canvas, x + 1, y + 2, "cooks %u/%u busy   queue %u", busy, k.cooksTotal,
                    queue);

        // One row per cook with a live progress bar.
        const int inner = width - 2;
        for (int i = 0; i < cookRows; ++i) {
            const int cy = y + 3 + i;
            caca_set_color_ansi(_canvas, CACA_DARKGRAY, CACA_BLACK);
            caca_printf(_canvas, x + 1, cy, "c%d", i);

            if (i >= static_cast<int>(k.cooks.size())) continue;
            const CookView& c = k.cooks[i];
            std::string label;
            if (c.state == protocol::CookState::Idle) {
                caca_set_color_ansi(_canvas, CACA_DARKGRAY, CACA_BLACK);
                caca_put_str(_canvas, x + 4, cy, "idle");
                continue;
            }
            label = std::string(pizza::typeToString(c.type)) + " " +
                    std::string(pizza::sizeToString(c.size));
            if (label.size() > 13) label = label.substr(0, 13);

            if (c.state == protocol::CookState::Waiting) {
                caca_set_color_ansi(_canvas, CACA_YELLOW, CACA_BLACK);
                caca_put_str(_canvas, x + 4, cy, label.c_str());
                caca_put_str(_canvas, x + 4 + 14, cy, "wait stock");
                continue;
            }

            // Baking: interpolate progress locally from the last sample.
            caca_set_color_ansi(_canvas, CACA_LIGHTGREEN, CACA_BLACK);
            caca_put_str(_canvas, x + 4, cy, label.c_str());

            double frac = 0.0;
            if (c.totalMs > 0) {
                double elapsed = static_cast<double>(c.elapsedMs) +
                                 static_cast<double>(nowMsVal - k.polledAtMs);
                frac = elapsed / static_cast<double>(c.totalMs);
            }
            if (frac < 0.0) frac = 0.0;
            if (frac > 1.0) frac = 1.0;

            const int barX = x + 4 + 14;
            const int barW = std::max(4, (x + 1 + inner) - barX - 5);
            const int fill = static_cast<int>(frac * barW + 0.5);
            caca_set_color_ansi(_canvas, CACA_DARKGRAY, CACA_BLACK);
            caca_put_char(_canvas, barX - 1, cy, '[');
            caca_put_char(_canvas, barX + barW, cy, ']');
            for (int b = 0; b < barW; ++b) {
                if (b < fill)
                    caca_set_color_ansi(_canvas, CACA_LIGHTGREEN, CACA_BLACK);
                else
                    caca_set_color_ansi(_canvas, CACA_DARKGRAY, CACA_BLACK);
                caca_put_char(_canvas, barX + b, cy, b < fill ? '#' : '-');
            }
            caca_set_color_ansi(_canvas, CACA_LIGHTGREEN, CACA_BLACK);
            caca_printf(_canvas, barX + barW + 2, cy, "%d%%", static_cast<int>(frac * 100));
        }

        // Stock line: nine abbreviated counters, red when depleted.
        const int sy = y + 3 + std::max(cookRows, 1);
        caca_set_color_ansi(_canvas, CACA_LIGHTGRAY, CACA_BLACK);
        caca_put_str(_canvas, x + 1, sy, "stk");
        int sx = x + 5;
        for (int i = 0; i < 9; ++i) {
            const uint32_t v = k.online ? k.stock[i] : 0;
            caca_set_color_ansi(_canvas, v == 0 ? CACA_LIGHTRED : CACA_LIGHTCYAN, CACA_BLACK);
            if (sx + 4 < x + width) {
                caca_printf(_canvas, sx, sy, "%s%u", kStockLabels[i], v);
                sx += 4;
            }
        }
        return h;
    }

    void Renderer::drawFooter(int width, int height)
    {
        const int sepY = height - kFooterRows;
        caca_set_color_ansi(_canvas, CACA_DARKGRAY, CACA_BLACK);
        for (int x = 0; x < width; ++x) caca_put_char(_canvas, x, sepY, '-');
        caca_put_str(_canvas, 2, sepY, " events ");

        std::vector<std::string> recent;
        {
            core::ScopedLock lock(&_eventMutex);
            const int want = 3;
            const int n = static_cast<int>(_events.size());
            for (int i = std::max(0, n - want); i < n; ++i) recent.push_back(_events[i]);
        }
        caca_set_color_ansi(_canvas, CACA_LIGHTGRAY, CACA_BLACK);
        for (std::size_t i = 0; i < recent.size(); ++i) {
            std::string line = recent[i];
            if (static_cast<int>(line.size()) > width - 2) line = line.substr(0, width - 2);
            caca_put_str(_canvas, 1, sepY + 1 + static_cast<int>(i), line.c_str());
        }

        // Input line.
        const int iy = height - 1;
        caca_set_color_ansi(_canvas, CACA_WHITE, CACA_BLACK);
        caca_put_str(_canvas, 0, iy, "order> ");
        std::string shown = _input;
        const int avail = width - 8;
        if (static_cast<int>(shown.size()) > avail && avail > 0)
            shown = shown.substr(shown.size() - avail);
        caca_set_color_ansi(_canvas, CACA_LIGHTGREEN, CACA_BLACK);
        caca_put_str(_canvas, 7, iy, shown.c_str());
        caca_put_char(_canvas, 7 + static_cast<int>(shown.size()), iy, '_');
    }

}  // namespace plazza::render

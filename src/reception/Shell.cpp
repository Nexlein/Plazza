/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Shell
*/

#include "plazza/reception/Shell.hpp"

#include <algorithm>
#include <chrono>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

#include "plazza/pizza/Pizza.hpp"
#include "plazza/reception/KitchenHandle.hpp"
#include "plazza/reception/Parser.hpp"

namespace plazza::reception {

    namespace {

        bool isBlank(const std::string& s)
        {
            for (char c : s) {
                if (c != ' ' && c != '\t' && c != '\r') return false;
            }
            return true;
        }

    }  // namespace

    Shell::Shell(Logger& logger, std::istream& in, std::ostream& out,
                 std::function<void(const Order&)> onOrder,
                 std::function<std::vector<KitchenHandle*>()> getKitchens)
        : _logger(logger), _in(in), _out(out), _onOrder(onOrder), _getKitchens(getKitchens)
    {
    }

    int Shell::run()
    {
        std::string line;
        while (true) {
            _out << "> ";
            _out.flush();
            if (!std::getline(_in, line)) break;
            if (isBlank(line)) continue;
            if (line == "exit") break;
            if (line == "status") {
                handleStatus();
                continue;
            }
            handleOrderLine(line);
        }
        return 0;
    }

    void Shell::handleStatus()
    {
        auto kitchens = _getKitchens();
        if (kitchens.empty()) {
            _out << "(no kitchens running)\n";
            _logger.info("status: no kitchens running");
            return;
        }

        for (auto* k : kitchens) k->triggerStatusQuery();

        struct Row {
            uint32_t id;
            pid_t    pid;
            bool     timedOut = false;
            uint32_t busy = 0, total = 0, outstanding = 0, queueDepth = 0;
            uint32_t stock[9]{};
        };

        auto start = std::chrono::steady_clock::now();
        std::vector<Row> rows;
        for (auto* k : kitchens) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
            auto timeout = std::chrono::milliseconds(80) - elapsed;
            if (timeout.count() <= 0) timeout = std::chrono::milliseconds(0);
            auto reply = k->collectStatusReply(timeout);

            Row r;
            r.id  = k->id();
            r.pid = k->pid();
            if (!reply.has_value()) {
                r.timedOut = true;
            } else {
                r.total       = reply->cooksTotal;
                r.outstanding = reply->outstanding;
                r.busy        = std::min(r.outstanding, r.total);
                r.queueDepth  = (r.outstanding > r.total) ? (r.outstanding - r.total) : 0;
                for (int i = 0; i < 9; ++i) r.stock[i] = reply->stock[i];
            }
            rows.push_back(r);
        }

        // ── layout (display character widths, not byte widths) ─────────────────
        constexpr int W_ID    = 3;
        constexpr int W_PID   = 7;
        constexpr int W_COOKS = 11;
        constexpr int W_PIZZA = 8;
        constexpr int W_ING   = 5;   // dgh/tom/gru/ham/msh/stk/egg  ×7
        constexpr int W_GOAT  = 6;
        constexpr int W_HEART = 3;
        constexpr int FULL_W  = W_ID + 1 + W_PID + 1 + W_COOKS + 1 + W_PIZZA
                            + 7 * (1 + W_ING) + 1 + W_GOAT + 1 + W_HEART; // 85

        // repeat a multibyte UTF-8 glyph n times
        auto rep = [](const char* s, int n) {
            std::string r; for (int i = 0; i < n; ++i) r += s; return r;
        };

        // pad ASCII string to display width w, centered or right
        auto padA = [](const std::string& s, int w, bool right = false) -> std::string {
            int sp = w - (int)s.size();
            if (sp <= 0) return s.substr(0, w);
            if (right) return std::string(sp, ' ') + s;
            int l = sp / 2;
            return std::string(l, ' ') + s + std::string(sp - l, ' ');
        };
        auto rpA = [&](const std::string& s, int w) { return padA(s, w, true); };

        // separator line
        auto mkSep = [&](const char* l, const char* m, const char* r) {
            auto seg = [&](int n) { return rep("─", n); };
            std::string s = l;
            s += seg(W_ID);    s += m;
            s += seg(W_PID);   s += m;
            s += seg(W_COOKS); s += m;
            s += seg(W_PIZZA); s += m;
            for (int i = 0; i < 7; ++i) { s += seg(W_ING); s += m; }
            s += seg(W_GOAT);  s += m;
            s += seg(W_HEART); s += r;
            return s;
        };

        // cook bar: always 4 blocks regardless of cook count
        auto cookBar = [](uint32_t busy) {
            std::string s;
            for (int i = 0; i < 4; ++i) s += (i < (int)busy) ? "█" : "░";
            return s;
        };

        // ── render ─────────────────────────────────────────────────────────────
        std::string title = "  status -- " + std::to_string(rows.size()) + " kitchen(s) active";
        title += std::string(std::max(0, FULL_W - (int)title.size()), ' ');

        _out << "┌" << rep("─", FULL_W) << "┐\n";
        _out << "│" << title             << "│\n";
        _out << mkSep("├", "┬", "┤")    << "\n";

        _out << "│" << padA(" #",     W_ID)
            << "│" << padA("PID",    W_PID)
            << "│" << padA("cooks",  W_COOKS)
            << "│" << padA("pizzas", W_PIZZA)
            << "│" << padA("dgh",    W_ING)
            << "│" << padA("tom",    W_ING)
            << "│" << padA("gru",    W_ING)
            << "│" << padA("ham",    W_ING)
            << "│" << padA("msh",    W_ING)
            << "│" << padA("stk",    W_ING)
            << "│" << padA("egg",    W_ING)
            << "│" << padA("goat",   W_GOAT)
            << "│" << " ♥ "   // display width 3 = W_HEART
            << "│\n";

        _out << mkSep("├", "┼", "┤") << "\n";

        for (auto& d : rows) {
            if (d.timedOut) {
                std::string msg = " #" + std::to_string(d.id)
                                + " (pid " + std::to_string(d.pid) + ") -- timed out";
                msg += std::string(std::max(0, FULL_W - (int)msg.size()), ' ');
                _out << "│" << msg << "│\n";
                continue;
            }

            // cooks cell: " ███░ 3/4  " — track display width manually (bar glyphs are multibyte)
            std::string fraction = std::to_string(d.busy) + "/" + std::to_string(d.total);
            std::string cooksCell = " " + cookBar(d.busy) + " " + fraction;
            int cooksDisp = 1 + 4 + 1 + (int)fraction.size();
            cooksCell += std::string(std::max(0, W_COOKS - cooksDisp), ' ');

            std::string pizzaStr = (d.outstanding == 0)
                ? "idle"
                : std::to_string(d.outstanding) + (d.queueDepth ? "+" + std::to_string(d.queueDepth) : "");

            _out << "│" << rpA(std::to_string(d.id),  W_ID   - 1) << " "
                << "│" << rpA(std::to_string(d.pid), W_PID  - 1) << " "
                << "│" << cooksCell
                << "│" << padA(pizzaStr, W_PIZZA)
                << "│" << rpA(std::to_string(d.stock[0]), W_ING  - 1) << " "
                << "│" << rpA(std::to_string(d.stock[1]), W_ING  - 1) << " "
                << "│" << rpA(std::to_string(d.stock[2]), W_ING  - 1) << " "
                << "│" << rpA(std::to_string(d.stock[3]), W_ING  - 1) << " "
                << "│" << rpA(std::to_string(d.stock[4]), W_ING  - 1) << " "
                << "│" << rpA(std::to_string(d.stock[5]), W_ING  - 1) << " "
                << "│" << rpA(std::to_string(d.stock[6]), W_ING  - 1) << " "
                << "│" << rpA(std::to_string(d.stock[7]), W_GOAT - 1) << " "
                << "│" << rpA(std::to_string(d.stock[8]), W_HEART- 1) << " "
                << "│\n";
        }

        _out << mkSep("└", "┴", "┘") << "\n";
        _out.flush();
        _logger.info("status command executed");
    }

    bool Shell::handleOrderLine(const std::string& line)
    {
        const auto parsed = Parser::parse(line);
        if (!parsed) {
            _out << "error: malformed order\n";
            _logger.warn("rejected order: " + line);
            return false;
        }
        std::size_t total = 0;
        for (const auto& order : *parsed) {
            total += order.count;
            _onOrder(order);
        }
        std::ostringstream msg;
        msg << "received: " << total << " pizza(s) accepted";
        _out << msg.str() << '\n';
        _logger.info("accepted order: " + line);
        return true;
    }

}  // namespace plazza::reception

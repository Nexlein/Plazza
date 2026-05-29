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

        // Trigger queries in parallel
        for (auto* k : kitchens) k->triggerStatusQuery();

        // Collect responses (limit total wait time to 80ms)
        auto start = std::chrono::steady_clock::now();
        _out << "Active Kitchens Status:\n";
        for (auto* k : kitchens) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start);
            auto timeout = std::chrono::milliseconds(80) - elapsed;
            if (timeout.count() <= 0) timeout = std::chrono::milliseconds(0);

            auto reply = k->collectStatusReply(timeout);
            _out << "Kitchen #" << k->id() << " (PID " << k->pid() << "):\n";
            if (!reply.has_value()) {
                _out << "  Status: (Query timed out or offline)\n";
                continue;
            }

            uint32_t total = reply->cooksTotal;
            uint32_t outstanding = reply->outstanding;
            uint32_t busy = std::min(outstanding, total);
            uint32_t idle = total - busy;
            uint32_t queueDepth = (outstanding > total) ? (outstanding - total) : 0;

            _out << "  Cooks: " << busy << " busy, " << idle << " idle (total " << total << ")\n";
            _out << "  Outstanding pizzas: " << outstanding << " (queue depth: " << queueDepth
                 << ")\n";
            _out << "  Ingredients stock:\n";
            for (std::size_t i = 0; i < 9; ++i) {
                auto ing = static_cast<plazza::pizza::Ingredient>(i);
                _out << "    " << plazza::pizza::ingredientToString(ing) << ": " << reply->stock[i]
                     << "\n";
            }
        }
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

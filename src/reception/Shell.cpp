/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Shell
*/

#include "plazza/reception/Shell.hpp"

#include <istream>
#include <ostream>
#include <sstream>
#include <string>

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

    Shell::Shell(Logger& logger, std::istream& in, std::ostream& out)
        : _logger(logger), _in(in), _out(out)
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
        _out << "(no kitchens running)\n";
        _logger.info("status: no kitchens running");
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
            _pending.push(order);
        }
        std::ostringstream msg;
        msg << "received: " << total << " pizza(s) accepted";
        _out << msg.str() << '\n';
        _logger.info("accepted order: " + line);
        return true;
    }

}  // namespace plazza::reception

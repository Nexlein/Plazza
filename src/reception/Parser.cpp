/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Parser
*/

#include "plazza/reception/Parser.hpp"

#include <cctype>
#include <cstddef>
#include <string>

namespace plazza::reception {

    namespace {

        bool isSpace(char c) { return c == ' ' || c == '\t'; }

        std::string_view trim(std::string_view s)
        {
            std::size_t begin = 0;
            std::size_t end = s.size();
            while (begin < end && isSpace(s[begin])) ++begin;
            while (end > begin && isSpace(s[end - 1])) --end;
            return s.substr(begin, end - begin);
        }

        std::vector<std::string_view> splitWhitespace(std::string_view s)
        {
            std::vector<std::string_view> tokens;
            std::size_t i = 0;
            while (i < s.size()) {
                while (i < s.size() && isSpace(s[i])) ++i;
                const std::size_t start = i;
                while (i < s.size() && !isSpace(s[i])) ++i;
                if (start < i) tokens.emplace_back(s.substr(start, i - start));
            }
            return tokens;
        }

        std::vector<std::string_view> splitSemicolon(std::string_view s)
        {
            std::vector<std::string_view> segments;
            std::size_t start = 0;
            for (std::size_t i = 0; i < s.size(); ++i) {
                if (s[i] == ';') {
                    segments.emplace_back(s.substr(start, i - start));
                    start = i + 1;
                }
            }
            segments.emplace_back(s.substr(start));
            return segments;
        }

        bool isAlphaWord(std::string_view s)
        {
            if (s.empty()) return false;
            for (char c : s) {
                if (!std::isalpha(static_cast<unsigned char>(c))) return false;
            }
            return true;
        }

        std::optional<std::size_t> parseCount(std::string_view s)
        {
            // grammar: x[1..9][0..9]*
            if (s.size() < 2) return std::nullopt;
            if (s[0] != 'x') return std::nullopt;
            if (s[1] < '1' || s[1] > '9') return std::nullopt;
            std::size_t value = static_cast<std::size_t>(s[1] - '0');
            for (std::size_t i = 2; i < s.size(); ++i) {
                if (s[i] < '0' || s[i] > '9') return std::nullopt;
                value = value * 10 + static_cast<std::size_t>(s[i] - '0');
            }
            return value;
        }

        std::optional<Order> parseSegment(std::string_view segment)
        {
            const auto trimmed = trim(segment);
            if (trimmed.empty()) return std::nullopt;
            const auto tokens = splitWhitespace(trimmed);
            if (tokens.size() != 3) return std::nullopt;

            if (!isAlphaWord(tokens[0])) return std::nullopt;
            const auto type = plazza::pizza::typeFromString(tokens[0]);
            if (!type) return std::nullopt;

            const auto size = plazza::pizza::sizeFromString(tokens[1]);
            if (!size) return std::nullopt;

            const auto count = parseCount(tokens[2]);
            if (!count) return std::nullopt;

            return Order{*type, *size, *count};
        }

    }  // namespace

    std::optional<std::vector<Order>> Parser::parse(std::string_view input)
    {
        const auto trimmed = trim(input);
        if (trimmed.empty()) return std::nullopt;

        const auto segments = splitSemicolon(trimmed);
        std::vector<Order> orders;
        orders.reserve(segments.size());
        for (const auto& segment : segments) {
            auto parsed = parseSegment(segment);
            if (!parsed) return std::nullopt;
            orders.push_back(*parsed);
        }
        return orders;
    }

}  // namespace plazza::reception

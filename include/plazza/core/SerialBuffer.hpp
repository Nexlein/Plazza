/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** SerialBuffer
*/

#pragma once

#include "plazza/reception/Order.hpp"
#include <vector>
#include <cstring>
#include <cstdint>

namespace plazza::core {

    class SerialBuffer {
        public:
        SerialBuffer() = default;
        ~SerialBuffer() = default;

        SerialBuffer& operator<<(const plazza::reception::Order& order);
        SerialBuffer& operator>>(plazza::reception::Order& order);

    private:
        template<typename T>
        inline void writeRaw(const T& val) {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&val);
            _buffer.insert(_buffer.end(), bytes, bytes + sizeof(T));
        }

        template<typename T>
        inline void readRaw(T& val) {
            std::memcpy(&val, _buffer.data() + _readPos, sizeof(T));
            _readPos += sizeof(T);
        }

        std::vector<uint8_t> _buffer;
        std::size_t _readPos = 0;
    };
}  // namespace plazza::core
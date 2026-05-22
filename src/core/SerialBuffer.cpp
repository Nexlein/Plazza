/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** SerialBuffer
*/

#include "plazza/core/SerialBuffer.hpp"

#include <cstring>

namespace plazza::core {
    SerialBuffer& SerialBuffer::operator<<(const plazza::reception::Order& order)
    {
        uint32_t start = static_cast<uint32_t>(_buffer.size());
        _buffer.resize(_buffer.size() + sizeof(uint32_t));

        writeRaw(order.type);
        writeRaw(order.size);
        writeRaw(order.count);

        uint32_t len = static_cast<uint32_t>(_buffer.size()) - start - sizeof(uint32_t);
        std::memcpy(_buffer.data() + start, &len, sizeof(uint32_t));
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(plazza::reception::Order& order)
    {
        uint32_t len;
        readRaw(len);

        readRaw(order.type);
        readRaw(order.size);
        readRaw(order.count);
        return *this;
    }
}  // namespace plazza::core
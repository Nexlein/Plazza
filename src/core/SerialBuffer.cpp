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

    SerialBuffer& SerialBuffer::operator<<(uint8_t value)
    {
        writeRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(uint8_t& value)
    {
        readRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator<<(uint16_t value)
    {
        writeRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(uint16_t& value)
    {
        readRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator<<(uint32_t value)
    {
        writeRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(uint32_t& value)
    {
        readRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator<<(std::size_t value)
    {
        uint64_t wire = static_cast<uint64_t>(value);
        writeRaw(wire);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(std::size_t& value)
    {
        uint64_t wire = 0;
        readRaw(wire);
        value = static_cast<std::size_t>(wire);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator<<(const std::string& value)
    {
        uint32_t len = static_cast<uint32_t>(value.size());
        writeRaw(len);
        if (len > 0)
            _buffer.insert(
                _buffer.end(), reinterpret_cast<const uint8_t*>(value.data()),
                reinterpret_cast<const uint8_t*>(value.data()) + len);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(std::string& value)
    {
        uint32_t len = 0;
        readRaw(len);
        value.assign(reinterpret_cast<const char*>(_buffer.data() + _readPos), len);
        _readPos += len;
        return *this;
    }

    SerialBuffer& SerialBuffer::operator<<(plazza::pizza::PizzaType value)
    {
        writeRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(plazza::pizza::PizzaType& value)
    {
        readRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator<<(plazza::pizza::PizzaSize value)
    {
        writeRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(plazza::pizza::PizzaSize& value)
    {
        readRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator<<(const plazza::pizza::Pizza& value)
    {
        writeRaw(value);
        return *this;
    }

    SerialBuffer& SerialBuffer::operator>>(plazza::pizza::Pizza& value)
    {
        readRaw(value);
        return *this;
    }
}  // namespace plazza::core
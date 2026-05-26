/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** SerialBuffer
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "plazza/pizza/Pizza.hpp"
#include "plazza/pizza/PizzaSize.hpp"
#include "plazza/pizza/PizzaType.hpp"
#include "plazza/reception/Order.hpp"

namespace plazza::core {

    class SerialBuffer {
        public:
        SerialBuffer() = default;
        ~SerialBuffer() = default;

        explicit SerialBuffer(std::vector<uint8_t> bytes) : _buffer(std::move(bytes)) {}

        const std::vector<uint8_t>& bytes() const { return _buffer; }
        std::size_t readPos() const { return _readPos; }
        bool exhausted() const { return _readPos >= _buffer.size(); }

        SerialBuffer& operator<<(const plazza::reception::Order& order);
        SerialBuffer& operator>>(plazza::reception::Order& order);

        SerialBuffer& operator<<(uint8_t value);
        SerialBuffer& operator>>(uint8_t& value);

        SerialBuffer& operator<<(uint16_t value);
        SerialBuffer& operator>>(uint16_t& value);

        SerialBuffer& operator<<(uint32_t value);
        SerialBuffer& operator>>(uint32_t& value);

        SerialBuffer& operator<<(std::size_t value);
        SerialBuffer& operator>>(std::size_t& value);

        SerialBuffer& operator<<(const std::string& value);
        SerialBuffer& operator>>(std::string& value);

        SerialBuffer& operator<<(plazza::pizza::PizzaType value);
        SerialBuffer& operator>>(plazza::pizza::PizzaType& value);

        SerialBuffer& operator<<(plazza::pizza::PizzaSize value);
        SerialBuffer& operator>>(plazza::pizza::PizzaSize& value);

        SerialBuffer& operator<<(const plazza::pizza::Pizza& value);
        SerialBuffer& operator>>(plazza::pizza::Pizza& value);

        private:
        template <typename T>
        inline void writeRaw(const T& val)
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&val);
            _buffer.insert(_buffer.end(), bytes, bytes + sizeof(T));
        }

        template <typename T>
        inline void readRaw(T& val)
        {
            std::memcpy(&val, _buffer.data() + _readPos, sizeof(T));
            _readPos += sizeof(T);
        }

        std::vector<uint8_t> _buffer;
        std::size_t _readPos = 0;
    };
}  // namespace plazza::core
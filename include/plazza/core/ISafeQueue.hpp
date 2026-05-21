/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** ISafeQueue
*/

#pragma once

namespace plazza::core {

    template <typename T>
    class ISafeQueue {
        public:
        virtual ~ISafeQueue() = default;

        virtual void push(T value) = 0;
        virtual bool tryPop(T& value) = 0;
        virtual T pop() = 0;
    };

}  // namespace plazza::core

/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Stock
*/

#pragma once

#include <array>
#include <atomic>
#include <cstddef>

#include "plazza/core/ConditionVariable.hpp"
#include "plazza/core/Mutex.hpp"
#include "plazza/pizza/Pizza.hpp"

namespace plazza::kitchen {

    class Stock {
        public:
        static constexpr std::size_t kInitialPerIngredient = 5;
        static constexpr std::size_t kIngredientCount = 9;

        Stock();

        Stock(const Stock&) = delete;
        Stock& operator=(const Stock&) = delete;

        bool tryConsume(const plazza::pizza::Recipe& recipe);
        bool waitAndConsume(
            const plazza::pizza::Recipe& recipe, const std::atomic<bool>& stop);

        void restockOnce();
        void notifyAll();

        std::size_t available(plazza::pizza::Ingredient ingredient) const;

        private:
        using Tally = std::array<std::size_t, kIngredientCount>;

        static Tally tally(const plazza::pizza::Recipe& recipe);
        bool hasEnoughLocked(const Tally& need) const;
        void consumeLocked(const Tally& need);

        mutable plazza::core::Mutex _mutex;
        plazza::core::ConditionVariable _cv;
        std::array<std::size_t, kIngredientCount> _counts;
    };

}  // namespace plazza::kitchen

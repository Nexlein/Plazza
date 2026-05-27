/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Stock
*/

#include "plazza/kitchen/Stock.hpp"

#include <chrono>

#include "plazza/core/ScopedLock.hpp"

namespace plazza::kitchen {

    Stock::Stock()
    {
        _counts.fill(kInitialPerIngredient);
    }

    Stock::Tally Stock::tally(const plazza::pizza::Recipe& recipe)
    {
        Tally need{};
        for (std::size_t k = 0; k < recipe.ingredientCount; ++k) {
            auto idx = static_cast<std::size_t>(recipe.ingredients[k]);
            if (idx < kIngredientCount) ++need[idx];
        }
        return need;
    }

    bool Stock::hasEnoughLocked(const Tally& need) const
    {
        for (std::size_t k = 0; k < kIngredientCount; ++k) {
            if (_counts[k] < need[k]) return false;
        }
        return true;
    }

    void Stock::consumeLocked(const Tally& need)
    {
        for (std::size_t k = 0; k < kIngredientCount; ++k) {
            _counts[k] -= need[k];
        }
    }

    bool Stock::tryConsume(const plazza::pizza::Recipe& recipe)
    {
        Tally need = tally(recipe);
        plazza::core::ScopedLock lock(&_mutex);
        if (!hasEnoughLocked(need)) return false;
        consumeLocked(need);
        return true;
    }

    bool Stock::waitAndConsume(
        const plazza::pizza::Recipe& recipe, const std::atomic<bool>& stop)
    {
        Tally need = tally(recipe);
        plazza::core::ScopedLock lock(&_mutex);
        while (!stop.load() && !hasEnoughLocked(need)) {
            _cv.waitFor(_mutex, std::chrono::milliseconds(50));
        }
        if (stop.load()) return false;
        consumeLocked(need);
        return true;
    }

    void Stock::restockOnce()
    {
        {
            plazza::core::ScopedLock lock(&_mutex);
            for (std::size_t k = 0; k < kIngredientCount; ++k) {
                ++_counts[k];
            }
        }
        _cv.notifyAll();
    }

    void Stock::notifyAll() { _cv.notifyAll(); }

    std::size_t Stock::available(plazza::pizza::Ingredient ingredient) const
    {
        plazza::core::ScopedLock lock(&_mutex);
        auto idx = static_cast<std::size_t>(ingredient);
        if (idx >= kIngredientCount) return 0;
        return _counts[idx];
    }

}  // namespace plazza::kitchen

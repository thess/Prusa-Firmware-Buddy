#pragma once

#include <array>
#include <exception> // for std::terminate

/// A wrapper over a std::array with some small extra features to work better for enum indexing:
/// - Initializer takes std::pair<Enum, Value> and checks that the Enum matches the array index (at compile time)
/// - Constructor is consteval and the initializer must be the same size as \p cnt
/// - Array now indexable with the enum type, no need to cast to_underlying
///
/// Functionally, this is equivalent to std::array<Value, cnt>.
template <typename Enum, typename Value, auto cnt>
struct EnumArray final : public std::array<Value, static_cast<size_t>(cnt)> {
    using Array = std::array<Value, static_cast<size_t>(cnt)>;

    constexpr EnumArray() noexcept {}

    explicit consteval EnumArray(std::initializer_list<std::pair<Enum, Value>> items) noexcept {
        // Check that the sizes match
        if (items.size() != static_cast<size_t>(cnt)) {
            std::terminate();
        }

        size_t i = 0;
        for (const auto &pair : items) {
            Array::operator[](i) = pair.second;

            // Check that indexes match
            if (static_cast<size_t>(pair.first) != i) {
                std::terminate();
            }

            i++;
        }
    }

    inline constexpr Value get_fallback(size_t i, size_t fallback_index) const {
        return Array::operator[](i < this->size() ? i : fallback_index);
    }
    inline constexpr Value get_fallback(Enum v, Enum fallback_value) const {
        return get_fallback(static_cast<size_t>(v), static_cast<size_t>(fallback_value));
    }

    inline constexpr const Value &operator[](size_t i) const {
        return Array::operator[](i);
    }
    inline constexpr const Value &operator[](Enum v) const {
        return Array::operator[](static_cast<size_t>(v));
    }

    inline constexpr Value &operator[](size_t i) {
        return Array::operator[](i);
    }
    inline constexpr Value &operator[](Enum v) {
        return Array::operator[](static_cast<size_t>(v));
    }
};

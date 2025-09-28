#pragma once
/**
 * @file tword.hpp
 * @brief Fixed-width ternary word types.
 */
#include "trit.hpp"
#include <array>
#include <span>
#include <string>

namespace t27::num {
/** 27-trit fixed-width word (LSB at index 0). */
struct Tword27 {
    static constexpr int width = 27;
    std::array<Trit, width> t{};

    [[nodiscard]] constexpr std::span<Trit, width> span() noexcept { return std::span<Trit, width>(t); }
    [[nodiscard]] constexpr std::span<const Trit, width> span() const noexcept { return std::span<const Trit, width>(t); }

    [[nodiscard]] std::string to_string() const {
        std::string s; s.reserve(width);
        for (int i = width - 1; i >= 0; --i) s.push_back(to_char(t[i]));
        size_t pos = s.find_first_not_of('0');
        if (pos == std::string::npos) return "0";
        return s.substr(pos);
    }
};
} // namespace t27::num
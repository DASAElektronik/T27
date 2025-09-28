#pragma once
/**
 * @file trit.hpp
 * @brief Basic Trit type and helpers for balanced ternary (−1,0,+1).
 */
#include <cstdint>
#include <compare>
#include <stdexcept>
#include <utility>

namespace t27::num {
/** Balanced ternary digit (LSB-first in vectors). */
enum class Trit : int8_t { N = -1, Z = 0, P = +1 };

constexpr auto operator<=>(Trit a, Trit b) noexcept { return static_cast<int>(a) <=> static_cast<int>(b); }
/// Unary negation on a Trit.
constexpr Trit neg(Trit t) noexcept { return static_cast<Trit>(-static_cast<int>(t)); }
/// Elementwise min/max on Trits (N < Z < P).
constexpr Trit tmin(Trit a, Trit b) noexcept { return (a <= b) ? a : b; }
constexpr Trit tmax(Trit a, Trit b) noexcept { return (a >= b) ? a : b; }

/// Convert ASCII char ('-','0','+') to Trit.
inline Trit from_char(char c) {
    switch (c) { case '-': return Trit::N; case '0': return Trit::Z; case '+': return Trit::P; default: throw std::invalid_argument("invalid trit char"); }
}
/// Convert Trit to ASCII char ('-','0','+').
constexpr char to_char(Trit t) noexcept { return (t == Trit::N) ? '-' : (t == Trit::Z) ? '0' : '+'; }

/**
 * @brief Full-adder for balanced ternary.
 * @param a First input trit
 * @param b Second input trit
 * @param cin Carry-in trit
 * @return (sum_trit, carry_out), each ∈ {−1,0,+1}
 */
constexpr inline std::pair<Trit, Trit> add_trit(Trit a, Trit b, Trit cin) noexcept {
    int s = static_cast<int>(a) + static_cast<int>(b) + static_cast<int>(cin);
    int carry = (s <= -2) ? -1 : (s >= 2 ? +1 : 0);
    int r = s - 3 * carry;
    return { static_cast<Trit>(r), static_cast<Trit>(carry) };
}
} // namespace t27::num
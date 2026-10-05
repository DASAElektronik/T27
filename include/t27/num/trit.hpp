// SPDX-License-Identifier: MIT
#pragma once
#include <compare>
#include <cstdint>
#include <stdexcept>
#include <utility>
namespace t27::num {
enum class Trit : std::int8_t { N = -1, Z = 0, P = 1 };
constexpr bool valid(Trit t) noexcept {
  return t == Trit::N || t == Trit::Z || t == Trit::P;
}
constexpr void require_valid(Trit t) {
  if (!valid(t))
    throw std::invalid_argument("invalid trit value");
}
constexpr auto operator<=>(Trit a, Trit b) noexcept {
  return static_cast<int>(a) <=> static_cast<int>(b);
}
constexpr Trit neg(Trit t) {
  require_valid(t);
  return static_cast<Trit>(-static_cast<int>(t));
}
constexpr Trit tmin(Trit a, Trit b) {
  require_valid(a);
  require_valid(b);
  return a < b ? a : b;
}
constexpr Trit tmax(Trit a, Trit b) {
  require_valid(a);
  require_valid(b);
  return a > b ? a : b;
}
inline Trit from_char(char c) {
  switch (c) {
  case '-':
    return Trit::N;
  case '0':
    return Trit::Z;
  case '+':
    return Trit::P;
  default:
    throw std::invalid_argument("expected '-', '0' or '+'");
  }
}
constexpr char to_char(Trit t) {
  require_valid(t);
  return t == Trit::N ? '-' : t == Trit::Z ? '0' : '+';
}
constexpr std::pair<Trit, Trit> add_trit(Trit a, Trit b, Trit cin) {
  require_valid(a);
  require_valid(b);
  require_valid(cin);
  const int sum = static_cast<int>(a) + static_cast<int>(b) + static_cast<int>(cin);
  const int carry = sum < -1 ? -1 : sum > 1 ? 1 : 0;
  return {static_cast<Trit>(sum - 3 * carry), static_cast<Trit>(carry)};
}
} // namespace t27::num

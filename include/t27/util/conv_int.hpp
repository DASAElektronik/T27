// SPDX-License-Identifier: MIT
#pragma once
#include "t27/num/convert.hpp"
#include "t27/num/detail.hpp"
#include <limits>
#include <string_view>
namespace t27::util {
using t27::num::Trit;
inline long long to_int64(std::span<const Trit> v) {
  return t27::num::from_bt(v);
}
inline std::vector<Trit> from_int64(long long x) {
  static_assert(sizeof(long long) == sizeof(std::int64_t));
  return t27::num::to_bt(static_cast<std::int64_t>(x));
}
// Same declarations, so using both namespaces is unambiguous.
using t27::num::parse_bt;
inline std::string to_string_bt(std::span<const Trit> v) {
  return t27::num::to_string(v);
}
/// Explicit legacy LSB-first format; empty string is rejected.
inline std::vector<Trit> parse_bt_lsb(std::string_view text) {
  return t27::num::parse_bt(std::string(text.rbegin(), text.rend()));
}
inline std::string to_string_bt_lsb(std::span<const Trit> v) {
  auto out = t27::num::to_string(v);
  return std::string(out.rbegin(), out.rend());
}
} // namespace t27::util

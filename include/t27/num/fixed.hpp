// SPDX-License-Identifier: MIT
#pragma once
#include "detail.hpp"
#include "ops.hpp"
#include "tword.hpp"
#include <algorithm>
#include <utility>
#include <vector>
namespace t27::num {
struct OpFlags {
  bool overflow{false}; ///< Exact integer result outside the 27-trit range.
  bool inexact{false};  ///< Right shift discarded a nonzero trit.
};
/// Explicit wrapping conversion modulo 3^27. Use pack_word27 to obtain flags.
inline Tword27 to_word27(std::span<const Trit> v) {
  detail::validate(v);
  Tword27 out{};
  for (std::size_t i = 0; i < std::min(v.size(), out.t.size()); ++i)
    out.t[i] = v[i];
  return out;
}
inline std::vector<Trit> to_vec(const Tword27 &w) {
  return detail::canonical(w.span());
}
inline std::pair<Tword27, OpFlags> pack_word27(std::span<const Trit> v) {
  auto out = to_word27(v);
  OpFlags flags{};
  for (std::size_t i = out.t.size(); i < v.size(); ++i)
    if (v[i] != Trit::Z)
      flags.overflow = true;
  return {out, flags};
}
inline std::pair<Tword27, OpFlags> add27(const Tword27 &a, const Tword27 &b) {
  return pack_word27(add(a.span(), b.span()));
}
inline std::pair<Tword27, OpFlags> sub27(const Tword27 &a, const Tword27 &b) {
  return pack_word27(sub(a.span(), b.span()));
}
inline std::pair<Tword27, OpFlags> neg27(const Tword27 &a) {
  return pack_word27(neg(a.span()));
}
inline std::pair<Tword27, OpFlags> mul27(const Tword27 &a, const Tword27 &b) {
  return pack_word27(mul(a.span(), b.span()));
}
inline std::pair<Tword27, OpFlags> shl27(const Tword27 &a, int k) {
  detail::require_shift(k);
  detail::validate(a.span());
  if (k >= Tword27::width)
    return {Tword27{}, {!detail::zero(a.span()), false}};
  return pack_word27(shl(a.span(), k));
}
inline std::pair<Tword27, OpFlags> lshr27(const Tword27 &a, int k) {
  detail::require_shift(k);
  detail::validate(a.span());
  OpFlags flags{};
  const auto count = std::min(static_cast<std::size_t>(k), a.t.size());
  for (std::size_t i = 0; i < count; ++i)
    if (a.t[i] != Trit::Z)
      flags.inexact = true;
  return {to_word27(lshr(a.span(), k)), flags};
}
/// Identical to lshr27: zero-filled trit shift, nearest division by odd 3^k.
inline std::pair<Tword27, OpFlags> shr27(const Tword27 &a, int k) {
  return lshr27(a, k);
}
} // namespace t27::num

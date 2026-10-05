// SPDX-License-Identifier: MIT
#pragma once
#include "fixed.hpp"
#include <algorithm>
namespace t27::num {
struct DivMod3kBalanced {
  std::vector<Trit> q, r;
};
inline DivMod3kBalanced divmod3k_balanced(std::span<const Trit> x, int k) {
  detail::require_shift(k);
  detail::validate(x);
  const auto count = std::min(static_cast<std::size_t>(k), x.size());
  return {lshr(x, k), detail::canonical(x.first(count))};
}
struct DivMod3kBalanced27 {
  Tword27 q, r;
  bool inexact{false};
};
inline DivMod3kBalanced27 divmod3k_balanced27(const Tword27 &a, int k) {
  const auto result = divmod3k_balanced(a.span(), k);
  return {to_word27(result.q), to_word27(result.r), !detail::zero(result.r)};
}
} // namespace t27::num

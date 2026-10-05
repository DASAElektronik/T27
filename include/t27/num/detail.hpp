// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "trit.hpp"
#include <span>
#include <vector>
namespace t27::num::detail {
inline void validate(std::span<const Trit> v) {
  for (Trit t : v)
    require_valid(t);
}
inline void canonicalize(std::vector<Trit> &v) {
  while (v.size() > 1 && v.back() == Trit::Z)
    v.pop_back();
  if (v.empty())
    v.push_back(Trit::Z);
}
inline std::vector<Trit> canonical(std::span<const Trit> v) {
  validate(v);
  if (v.empty())
    return {Trit::Z};
  auto count = v.size();
  while (count > 1 && v[count - 1] == Trit::Z)
    --count;
  const auto significant = v.first(count);
  return std::vector<Trit>(significant.begin(), significant.end());
}
inline bool zero(std::span<const Trit> v) {
  validate(v);
  for (Trit t : v)
    if (t != Trit::Z)
      return false;
  return true;
}
inline void require_shift(int k) {
  if (k < 0)
    throw std::invalid_argument("negative trit shift/exponent");
}
} // namespace t27::num::detail

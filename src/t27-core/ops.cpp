// SPDX-License-Identifier: MIT
#include "t27/num/ops.hpp"
#include "t27/num/detail.hpp"
#include <algorithm>
namespace t27::num {
std::vector<Trit> add(std::span<const Trit> a, std::span<const Trit> b) {
  detail::validate(a);
  detail::validate(b);
  std::vector<Trit> out;
  const auto size = std::max(a.size(), b.size());
  out.reserve(size);
  Trit carry = Trit::Z;
  for (std::size_t i = 0; i < size; ++i) {
    auto [digit, next] =
        add_trit(i < a.size() ? a[i] : Trit::Z, i < b.size() ? b[i] : Trit::Z, carry);
    out.push_back(digit);
    carry = next;
  }
  if (carry != Trit::Z)
    out.push_back(carry);
  detail::canonicalize(out);
  return out;
}
std::vector<Trit> neg(std::span<const Trit> a) {
  std::vector<Trit> out;
  out.reserve(a.size());
  for (Trit t : a)
    out.push_back(neg(t));
  detail::canonicalize(out);
  return out;
}
std::vector<Trit> sub(std::span<const Trit> a, std::span<const Trit> b) {
  return add(a, neg(b));
}
Trit cmp(std::span<const Trit> a, std::span<const Trit> b) {
  detail::validate(a);
  detail::validate(b);
  for (std::size_t i = std::max(a.size(), b.size()); i-- > 0;) {
    const auto x = i < a.size() ? a[i] : Trit::Z;
    const auto y = i < b.size() ? b[i] : Trit::Z;
    if (x != y)
      return x < y ? Trit::N : Trit::P;
  }
  return Trit::Z;
}
std::vector<Trit> shl(std::span<const Trit> a, int k) {
  detail::require_shift(k);
  auto value = detail::canonical(a);
  if (detail::zero(value) || k == 0)
    return value;
  const auto count = static_cast<std::size_t>(k);
  if (count > value.max_size() - value.size())
    throw std::length_error("shift too large");
  value.insert(value.begin(), count, Trit::Z);
  return value;
}
std::vector<Trit> lshr(std::span<const Trit> a, int k) {
  detail::require_shift(k);
  detail::validate(a);
  const auto count = static_cast<std::size_t>(k);
  if (count >= a.size())
    return {Trit::Z};
  return detail::canonical(a.subspan(count));
}
std::vector<Trit> shr(std::span<const Trit> a, int k) {
  return lshr(a, k);
}
std::vector<Trit> mul(std::span<const Trit> a, std::span<const Trit> b) {
  auto term = detail::canonical(a);
  auto multiplier = detail::canonical(b);
  if (detail::zero(term) || detail::zero(multiplier))
    return {Trit::Z};
  std::vector<Trit> result{Trit::Z};
  for (std::size_t i = 0; i < multiplier.size(); ++i) {
    if (multiplier[i] == Trit::P)
      result = add(result, term);
    if (multiplier[i] == Trit::N)
      result = sub(result, term);
    if (i + 1 < multiplier.size())
      term.insert(term.begin(), Trit::Z);
  }
  return result;
}
} // namespace t27::num

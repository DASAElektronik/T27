// SPDX-License-Identifier: MIT
#pragma once
#include "detail.hpp"
#include "div.hpp"
namespace t27::num {
/** Exact shifted-divisor long division on balanced-trit vectors.
 * Truncation toward zero; no conversion to a machine integer, no fallback.
 * See docs/proof_long_div.md for invariant and termination argument.
 */
inline DivMod divmod_long(std::span<const Trit> a, std::span<const Trit> b) {
  auto dividend = detail::canonical(a);
  auto divisor = detail::canonical(b);
  if (detail::zero(divisor))
    throw std::invalid_argument("division by zero");
  if (detail::zero(dividend))
    return {{Trit::Z}, {Trit::Z}};
  const bool negative_a = dividend.back() == Trit::N;
  const bool negative_b = divisor.back() == Trit::N;
  if (negative_a)
    dividend = neg(dividend);
  if (negative_b)
    divisor = neg(divisor);
  const std::size_t shift =
      dividend.size() >= divisor.size() ? dividend.size() - divisor.size() + 1 : 0;
  auto trial = divisor;
  trial.insert(trial.begin(), shift, Trit::Z);
  std::vector<Trit> quotient{Trit::Z};
  auto remainder = dividend;
  for (std::size_t position = shift + 1; position-- > 0;) {
    std::vector<Trit> unit(position + 1, Trit::Z);
    unit.back() = Trit::P;
    // At entry, 0 <= R < 3*trial. At most two subtractions suffice.
    for (int digit = 0; digit < 2 && cmp(remainder, trial) != Trit::N; ++digit) {
      remainder = sub(remainder, trial);
      quotient = add(quotient, unit);
    }
    if (cmp(remainder, trial) != Trit::N)
      throw std::logic_error("long division invariant violated");
    if (position > 0)
      trial.erase(trial.begin());
  }
  if (negative_a != negative_b)
    quotient = neg(quotient);
  if (negative_a)
    remainder = neg(remainder);
  return {std::move(quotient), std::move(remainder)};
}
} // namespace t27::num

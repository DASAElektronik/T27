// SPDX-License-Identifier: MIT
#pragma once
/**
 * @file div.hpp
 * @brief General balanced-ternary division (long division) + rounding variants.
 */
#include "fixed.hpp"
#include "ops.hpp"
#include <stdexcept>
#include <utility>

namespace t27::num {

/** Truncating quotient and remainder: a=q*b+r, |r|<|b|, r has the sign of a. */
struct DivMod {
  std::vector<Trit> q, r;
};

/** Exact shifted-divisor long division, without a machine-integer fallback. */
DivMod divmod(std::span<const Trit> a, std::span<const Trit> b);

// --- Helpers ---
inline std::vector<Trit> abs_vec(std::span<const Trit> v) {
  if (cmp(v, std::vector<Trit>{Trit::Z}) == Trit::N)
    return neg(v);
  return detail::canonical(v);
}
inline Trit sgn(std::span<const Trit> v) {
  return cmp(v, std::vector<Trit>{Trit::Z});
}
inline bool is_zero(std::span<const Trit> v) {
  return sgn(v) == Trit::Z;
}

/** Euclidean division: r >= 0 and r < |b|. */
struct DivRR {
  std::vector<Trit> q, r;
};
DivRR divmod_euclid(std::span<const Trit> a, std::span<const Trit> b);
/** Floor division: q = floor(a/b). */
DivRR divmod_floor(std::span<const Trit> a, std::span<const Trit> b);
/** Ceil division: q = ceil(a/b). */
DivRR divmod_ceil(std::span<const Trit> a, std::span<const Trit> b);
/** Truncation division: q = trunc(a/b) (toward zero). */
DivRR divmod_trunc(std::span<const Trit> a, std::span<const Trit> b);
/** Nearest-integer division, ties away from zero. */
DivRR divmod_nearest_away(std::span<const Trit> a, std::span<const Trit> b);
/** Nearest-integer division, ties to even. */
DivRR divmod_nearest_even(std::span<const Trit> a, std::span<const Trit> b);

// ---- 27T wrappers ----
struct DivMod27 {
  Tword27 q;
  Tword27 r;
  bool divide_by_zero{false};
  bool overflow_q{false};
  bool inexact{false};
};
DivMod27 divmod27(const Tword27 &a, const Tword27 &b);

struct DivRR27 {
  Tword27 q, r;
  bool divide_by_zero{false};
  bool overflow_q{false};
  bool inexact{false};
};
DivRR27 divmod_euclid27(const Tword27 &a, const Tword27 &b);
DivRR27 divmod_floor27(const Tword27 &a, const Tword27 &b);
DivRR27 divmod_ceil27(const Tword27 &a, const Tword27 &b);
DivRR27 divmod_trunc27(const Tword27 &a, const Tword27 &b);
DivRR27 divmod_nearest_away27(const Tword27 &a, const Tword27 &b);
DivRR27 divmod_nearest_even27(const Tword27 &a, const Tword27 &b);

} // namespace t27::num
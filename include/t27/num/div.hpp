/**\file
 * Division front-ends and rounding modes (incl. 27-trit wrappers).
 */
#pragma once
/**
 * @file div.hpp
 * @brief General balanced-ternary division (long division) + rounding variants.
 */
#include "ops.hpp"
#include "fixed.hpp"
#include <utility>
#include <stdexcept>

namespace t27::num {

/** Result of a/b with balanced remainder r s.t. a = q*b + r and |r| < |b|. */
struct DivMod { std::vector<Trit> q, r; };

/** Long division in balanced ternary: greedy cancellation of leading trit. */
DivMod divmod(std::span<const Trit> a, std::span<const Trit> b);

// --- Helpers ---
inline std::vector<Trit> abs_vec(std::span<const Trit> v){
    if (cmp(v, std::vector<Trit>{Trit::Z}) == Trit::N) return neg(v);
    return std::vector<Trit>(v.begin(), v.end());
}
inline Trit /** Sign of a vector: most significant non-zero trit as N/Z/P. */
sgn(std::span<const Trit> v){ return cmp(v, std::vector<Trit>{Trit::Z}); }
inline bool /** True iff vector is zero (all trits Z or empty). */
is_zero(std::span<const Trit> v){ return sgn(v) == Trit::Z; }

/** Euclidean division: r >= 0 and r < |b|. */
struct DivRR { std::vector<Trit> q, r; };
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
struct DivMod27 { Tword27 q; Tword27 r; bool divide_by_zero{false}; bool overflow_q{false}; bool inexact{false}; };
DivMod27 divmod27(const Tword27& a, const Tword27& b);

struct DivRR27 { Tword27 q, r; bool divide_by_zero{false}; bool overflow_q{false}; };
DivRR27 /** 27-trit Euclidean division (r >= 0, r < |b|). */
divmod_euclid27(const Tword27& a, const Tword27& b);
DivRR27 /** 27-trit floor division (q = floor(a/b)). */
divmod_floor27(const Tword27& a, const Tword27& b);
DivRR27 /** 27-trit ceil division (q = ceil(a/b)). */
divmod_ceil27(const Tword27& a, const Tword27& b);
DivRR27 /** 27-trit truncation division (towards zero). */
divmod_trunc27(const Tword27& a, const Tword27& b);
DivRR27 /** 27-trit nearest-integer division (ties away from zero). */
divmod_nearest_away27(const Tword27& a, const Tword27& b);
DivRR27 /** 27-trit nearest-integer division (ties-to-even). */
divmod_nearest_even27(const Tword27& a, const Tword27& b);

} // namespace t27::num
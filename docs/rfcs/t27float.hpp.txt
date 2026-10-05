// Copyright 2025 T27
// SPDX-License-Identifier: Apache-2.0
//
// Balanced-ternary floating point: Ternary27 (draft) and a tiny Ternary6 helper.
// This header only defines data containers and helper predicates. Pack/convert
// are declared for later implementation.
//
// Note: keep this header ASCII-only to avoid encoding issues on some CI runners.

#pragma once
#include <array>
#include <cstdint>

namespace t27 {

// A balanced trit is represented by int8_t values -1, 0, +1.
using trit = int8_t;

// Two-trit type code for floating point values.
enum class fp_type : uint8_t {
  // Specials
  Inf_pp,        // "++"  infinity
  QNaN_p0,       // "+0"  quiet NaN
  SNaN_pn,       // "+-"  signaling NaN

  // Reals with rounding annotation (down/exact/up)
  Real_0n,       // "0-"  rounded down
  Real_00,       // "00"  exact
  Real_0p,       // "0+"  rounded up

  // Reserved codes for future use
  Res_np,
  Res_n0,
  Res_nn
};

// Canonical 27-trit float container (field-wise, not packed).
struct tfloat27 {
  fp_type type;                 // 2 trits as enumerated
  trit    sign;                 // -1, 0, +1  (0 indicates subnormal reals)
  std::array<trit, 5>  exp;     // exponent (balanced digits)
  std::array<trit, 19> sig;     // significand (balanced digits)
};

// Tiny 6-trit float used for testing.
struct tfloat6 {
  fp_type type;                 // 2 trits
  trit    sign;                 // 1 trit
  std::array<trit, 2> exp;      // 2 trits
  trit    sig;                  // 1 trit
};

// Predicates
inline bool is_nan(fp_type t)      { return t == fp_type::QNaN_p0 || t == fp_type::SNaN_pn; }
inline bool is_qnan(fp_type t)     { return t == fp_type::QNaN_p0; }
inline bool is_snan(fp_type t)     { return t == fp_type::SNaN_pn; }
inline bool is_inf(fp_type t)      { return t == fp_type::Inf_pp; }
inline bool is_real(fp_type t)     { return t == fp_type::Real_0n || t == fp_type::Real_00 || t == fp_type::Real_0p; }
inline bool is_subnormal(const tfloat27& x) { return is_real(x.type) && x.sign == 0; }

// Pack to a 64-bit host word using base-3 digits 0..2 (offset encoding trit+1).
// The exact digit order is a library ABI detail; tests should use pack+unpack pairs.
uint64_t pack27_to_u64(const tfloat27& v);
tfloat27  unpack27_from_u64(uint64_t word);

// Reference conversions (subset to start with).
// These will stabilize as the spec is refined.
tfloat27 from_double(double x);
double   to_double(const tfloat27& v);

// Comparison result: -1, 0, +1  (NaN rules TBD; likely unordered returns 0 and sets a flag)
int cmp_total_order(const tfloat27& a, const tfloat27& b);

} // namespace t27

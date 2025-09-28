/**\file
 * Primitive arithmetic on balanced-ternary vectors.
 */
#pragma once
/**
 * @file ops.hpp
 * @brief Core arithmetic and shifts on balanced-ternary vectors.
 */
#include "trit.hpp"
#include <vector>
#include <span>
#include <utility>

namespace t27::num {
std::vector<Trit> /** Add two balanced-ternary vectors (LSB-first). Canonicalizes the result. */
add(std::span<const Trit> a, std::span<const Trit> b);
std::vector<Trit> /** Unary minus (element-wise negation + canonicalization). */
neg(std::span<const Trit> a);
std::vector<Trit> /** Subtraction a - b implemented as add(a, neg(b)). */
sub(std::span<const Trit> a, std::span<const Trit> b);
Trit /** Three-way compare (N if a<b, Z if a==b, P if a>b). */
cmp(std::span<const Trit> a, std::span<const Trit> b);
std::vector<Trit> /** Arithmetic left shift by k trits (multiply by 3^k). */
shl(std::span<const Trit> a, int k);
std::vector<Trit> /** Arithmetic right shift by k trits (divide by 3^k, sign-preserving). */
shr(std::span<const Trit> a, int k);
std::vector<Trit> /** Logical right shift by k trits (drop k LSB trits, pad zeros). */
lshr(std::span<const Trit> a, int k);
std::vector<Trit> /** Schoolbook multiplication with canonicalized result. */
mul(std::span<const Trit> a, std::span<const Trit> b);
} // namespace t27::num
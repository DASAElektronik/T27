// SPDX-License-Identifier: MIT
#pragma once
/**
 * @file ops.hpp
 * @brief Core arithmetic and shifts on balanced-ternary vectors.
 */
#include "trit.hpp"
#include <span>
#include <utility>
#include <vector>

namespace t27::num {
std::vector<Trit> add(std::span<const Trit> a, std::span<const Trit> b);
std::vector<Trit> neg(std::span<const Trit> a);
std::vector<Trit> sub(std::span<const Trit> a, std::span<const Trit> b);
Trit cmp(std::span<const Trit> a, std::span<const Trit> b);
std::vector<Trit> shl(std::span<const Trit> a, int k);
std::vector<Trit> shr(std::span<const Trit> a, int k);
std::vector<Trit> lshr(std::span<const Trit> a, int k);
std::vector<Trit> mul(std::span<const Trit> a, std::span<const Trit> b);
} // namespace t27::num
// SPDX-License-Identifier: MIT
#pragma once
/**
 * @file convert.hpp
 * @brief Conversions between int64 and balanced ternary vectors.
 */
#include "trit.hpp"
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace t27::num {
std::vector<Trit> to_bt(int64_t n);
int64_t from_bt(std::span<const Trit> trits);
std::string to_string(std::span<const Trit> trits);
std::vector<Trit> parse_bt(std::string_view ms_to_ls);
} // namespace t27::num
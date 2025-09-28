/**\file
 * Integer <-> balanced-ternary conversions and +0- formatted strings.
 */
#pragma once
/**
 * @file convert.hpp
 * @brief Conversions between int64 and balanced ternary vectors.
 */
#include "trit.hpp"
#include <vector>
#include <span>
#include <cstdint>
#include <string>
#include <string_view>

namespace t27::num {
std::vector<Trit> /** Convert signed 64-bit integer to balanced-ternary vector (LSB-first).
 *  \param n value to convert
 *  \return little-endian trit vector where index i encodes coefficient of 3^i
 */
to_bt(int64_t n);
int64_t /** Convert balanced-ternary vector (LSB-first) to signed 64-bit integer.
 *  \throws std::overflow_error if value does not fit into int64
 */
from_bt(std::span<const Trit> trits);
std::string /** Compact “+0-” string for a trit vector (LSB-first). Zero -> "0". */
to_string(std::span<const Trit> trits);
std::vector<Trit> /** Parse compact “+0-” string (LSB-first) into a trit vector.
 *  Whitespace is ignored. Trailing MS zeros are stripped; "0" -> empty vector.
 *  \throws std::invalid_argument on illegal characters
 */
parse_bt(std::string_view ms_to_ls);
} // namespace t27::num
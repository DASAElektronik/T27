// SPDX-License-Identifier: MIT
#include "t27/num/convert.hpp"
#include "t27/num/detail.hpp"
#include <limits>
namespace t27::num {
std::vector<Trit> to_bt(std::int64_t n) {
  std::vector<Trit> out;
  do {
    auto quotient = n / 3;
    auto remainder = n % 3;
    if (remainder == 2) {
      remainder = -1;
      ++quotient;
    }
    if (remainder == -2) {
      remainder = 1;
      --quotient;
    }
    out.push_back(static_cast<Trit>(remainder));
    n = quotient;
  } while (n != 0);
  return out;
}
std::int64_t from_bt(std::span<const Trit> trits) {
  detail::validate(trits);
  std::size_t length = trits.size();
  while (length && trits[length - 1] == Trit::Z)
    --length;
  if (!length)
    return 0;
  const bool negative = trits[length - 1] == Trit::N;
  const auto max = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
  const std::uint64_t limit = negative ? max + 1 : max;
  std::uint64_t magnitude = 0;
  // Positive balanced prefixes are >= 1 and grow monotonically. Check the
  // complete 3*m+d operation before multiplication; no signed overflow.
  for (std::size_t i = length; i-- > 0;) {
    const int digit = static_cast<int>(trits[i]) * (negative ? -1 : 1);
    const auto threshold =
        digit < 0 ? (limit + 1) / 3 : (limit - static_cast<std::uint64_t>(digit)) / 3;
    if (magnitude > threshold)
      throw std::overflow_error("balanced value outside int64 range");
    magnitude *= 3;
    if (digit < 0)
      --magnitude;
    else
      magnitude += static_cast<std::uint64_t>(digit);
  }
  if (!negative)
    return static_cast<std::int64_t>(magnitude);
  if (magnitude == max + 1)
    return std::numeric_limits<std::int64_t>::min();
  return -static_cast<std::int64_t>(magnitude);
}
std::string to_string(std::span<const Trit> trits) {
  auto value = detail::canonical(trits);
  std::string out;
  out.reserve(value.size());
  for (auto it = value.rbegin(); it != value.rend(); ++it)
    out.push_back(to_char(*it));
  return out;
}
std::vector<Trit> parse_bt(std::string_view text) {
  if (text.empty())
    throw std::invalid_argument("empty balanced-ternary string");
  std::vector<Trit> out;
  out.reserve(text.size());
  for (auto it = text.rbegin(); it != text.rend(); ++it)
    out.push_back(from_char(*it));
  detail::canonicalize(out);
  return out;
}
} // namespace t27::num

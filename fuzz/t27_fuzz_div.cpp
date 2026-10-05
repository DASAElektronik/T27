// SPDX-License-Identifier: MIT
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <t27/num/div_long.hpp>
using namespace t27::num;
static std::vector<Trit> trits(const std::uint8_t *bytes, std::size_t length) {
  std::vector<Trit> out;
  for (std::size_t i = 0; i < length && i < 128; ++i)
    out.push_back(static_cast<Trit>(int(bytes[i] % 3) - 1));
  detail::canonicalize(out);
  return out;
}
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
  if (size == 0)
    return 0;
  const auto half = size / 2;
  auto a = trits(data, half), b = trits(data + half, size - half);
  if (detail::zero(b))
    b = {Trit::P};
  auto [q, r] = divmod_long(a, b);
  if (cmp(add(mul(q, b), r), a) != Trit::Z)
    std::abort();
  if (cmp(abs_vec(r), abs_vec(b)) != Trit::N)
    std::abort();
  if (!is_zero(r) && sgn(r) != sgn(a))
    std::abort();
  return 0;
}

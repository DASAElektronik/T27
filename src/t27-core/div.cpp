// SPDX-License-Identifier: MIT
#include "t27/num/div_long.hpp"
namespace t27::num {
DivMod divmod(std::span<const Trit> a, std::span<const Trit> b) {
  return divmod_long(a, b);
}
DivMod27 divmod27(const Tword27 &a, const Tword27 &b) {
  detail::validate(a.span());
  detail::validate(b.span());
  if (detail::zero(b.span()))
    return {{}, {}, true, false, false};
  auto result = divmod(a.span(), b.span());
  auto [q, flags] = pack_word27(result.q);
  return {q, to_word27(result.r), false, flags.overflow, !detail::zero(result.r)};
}
} // namespace t27::num

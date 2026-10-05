// SPDX-License-Identifier: MIT
#pragma once
#include "div_long.hpp"
namespace t27::num {
/// Compatibility entry point: all division is now strict and fallback-free.
inline DivMod divmod_long_strict(std::span<const Trit> a, std::span<const Trit> b) {
  return divmod_long(a, b);
}
} // namespace t27::num

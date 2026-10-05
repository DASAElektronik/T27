// SPDX-License-Identifier: MIT
#pragma once
/**
 * \file
 * \ingroup numerics
 * \brief Small pretty-printers for balanced-trit vectors.
 */
#include <span>
#include <sstream>
#include <string>
#include <t27/num/convert.hpp>
#include <t27/num/fixed.hpp>
#include <t27/num/trit.hpp>
#include <vector>

namespace t27 {
namespace util {

using t27::num::Trit;
using t27::num::Tword27;

/** Compact canonical MSB-first '+0-' string. */
inline std::string str_compact(std::span<const Trit> v) {
  return t27::num::to_string(v);
}

/** Pretty string with powers: "+ 3^0, - 3^2, ..." for debugging. */
inline std::string str_expanded(std::span<const Trit> v) {
  t27::num::detail::validate(v);
  std::ostringstream oss;
  bool first = true;
  for (size_t i = 0; i < v.size(); ++i) {
    int d = (v[i] == Trit::P ? +1 : (v[i] == Trit::N ? -1 : 0));
    if (!d)
      continue;
    if (!first)
      oss << ", ";
    oss << (d > 0 ? "+ " : "- ") << "3^" << i;
    first = false;
  }
  if (first)
    return "0";
  return oss.str();
}

} // namespace util
} // namespace t27
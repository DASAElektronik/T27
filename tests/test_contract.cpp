// SPDX-License-Identifier: Apache-2.0
#include "t27/num/div3k.hpp"
#include "t27/num/div_long_strict.hpp"
#include "t27/util/conv_int.hpp"
#include "t27/util/print.hpp"
#include "test_support.hpp"
#include <iostream>
#include <limits>
using namespace t27::num;
static Tword27 word(std::int64_t n) {
  return to_word27(to_bt(n));
}
static std::int64_t value(const Tword27 &w) {
  return from_bt(w.span());
}
int main() try {
  for (int a = -1; a <= 1; ++a)
    for (int b = -1; b <= 1; ++b)
      for (int c = -1; c <= 1; ++c) {
        auto [s, carry] =
            add_trit(static_cast<Trit>(a), static_cast<Trit>(b), static_cast<Trit>(c));
        CHECK(valid(s) && valid(carry));
        CHECK(int(s) + 3 * int(carry) == a + b + c);
      }
  std::vector<Trit> empty, zero{Trit::Z}, padded{Trit::P, Trit::Z, Trit::Z};
  CHECK(add(empty, empty) == zero);
  CHECK(neg(empty) == zero);
  CHECK(shl(empty, 20) == zero);
  CHECK(lshr(empty, 0) == zero);
  CHECK(divmod3k_balanced(empty, 0).r == zero);
  CHECK(to_string(padded) == "+");
  CHECK(shl(padded, 0) == to_bt(1));
  CHECK(t27::util::parse_bt("+0") == to_bt(3));
  CHECK(t27::util::parse_bt_lsb("+0") == to_bt(1));
  CHECK(t27::util::to_string_bt(to_bt(3)) == "+0");
  CHECK(t27::util::from_int64(0) == zero);
  for (auto n :
       {std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()})
    CHECK(from_bt(to_bt(n)) == n);
  check_throws<std::overflow_error>(
      [] { from_bt(add(to_bt(std::numeric_limits<std::int64_t>::max()), to_bt(1))); });
  check_throws<std::overflow_error>(
      [] { from_bt(sub(to_bt(std::numeric_limits<std::int64_t>::min()), to_bt(1))); });
  for (auto s : {"", "a+", "+ ", "1", "--x"})
    check_throws<std::invalid_argument>([&] { parse_bt(s); });
  check_throws<std::invalid_argument>([] { shl(to_bt(1), -1); });
  check_throws<std::invalid_argument>([] { shr(to_bt(1), -1); });
  check_throws<std::invalid_argument>([] { shl27(word(1), -1); });
  check_throws<std::invalid_argument>([] { divmod3k_balanced(to_bt(1), -1); });
  std::vector<Trit> invalid{static_cast<Trit>(2)};
  check_throws<std::invalid_argument>([&] { add(invalid, zero); });
  check_throws<std::invalid_argument>([&] { cmp(invalid, zero); });
  check_throws<std::invalid_argument>([&] { from_bt(invalid); });
  check_throws<std::invalid_argument>([&] { divmod(invalid, zero); });
  check_throws<std::invalid_argument>([&] { divmod(zero, zero); });
  CHECK(divmod27(word(1), word(0)).divide_by_zero);
  using Round27 = DivRR27 (*)(const Tword27 &, const Tword27 &);
  for (Round27 fn : {divmod_euclid27, divmod_floor27, divmod_ceil27, divmod_trunc27,
                     divmod_nearest_away27, divmod_nearest_even27}) {
    CHECK(fn(word(1), word(0)).divide_by_zero);
    CHECK(fn(word(5), word(2)).inexact);
    CHECK(!fn(word(4), word(2)).inexact);
  }
  constexpr std::int64_t maximum = 3812798742493LL;
  auto wrapped = add27(word(maximum), word(1));
  CHECK(value(wrapped.first) == -maximum && wrapped.second.overflow);
  CHECK(value(neg27(word(-maximum)).first) == maximum);
  auto m = mul27(word(2541865828329LL), word(3));
  CHECK(value(m.first) == 0 && m.second.overflow);
  CHECK(value(shr27(word(3), 1).first) == 1);
  CHECK(shr27(word(4), 1).second.inexact && !shr27(word(4), 1).second.overflow);
  CHECK(!shr27(word(3), 1).second.inexact);
  CHECK(value(shl27(word(1), std::numeric_limits<int>::max()).first) == 0);
  CHECK(shr27(word(1), std::numeric_limits<int>::max()).second.inexact);
  auto dm = divmod_long_strict(to_bt(5), to_bt(4));
  CHECK(from_bt(dm.q) == 1 && from_bt(dm.r) == 1);
  // Beyond int64: construct a known quotient/remainder, not a round trip via divmod.
  auto big_q = add(shl(to_bt(1), 160), to_bt(11));
  auto big_b = add(shl(to_bt(1), 80), to_bt(17));
  auto big_a = add(mul(big_q, big_b), to_bt(23));
  auto big = divmod_long_strict(big_a, big_b);
  CHECK(big.q == big_q && big.r == to_bt(23));
  std::cout << "Contract checks passed (active with NDEBUG).\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}

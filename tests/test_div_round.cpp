// SPDX-License-Identifier: MIT
#include "t27/num/convert.hpp"
#include "t27/num/div.hpp"
#include "test_support.hpp"
#include <cstdlib>
#include <iostream>
#include <random>
using namespace t27::num;
static void check_rounding(long long a, long long b) {
  const auto A = to_bt(a), B = to_bt(b);
  const long long qt = a / b, rt = a % b;
  const long long direction = (a < 0) == (b < 0) ? 1 : -1;
  const auto twice = 2 * std::llabs(rt), denominator = std::llabs(b);
  auto check = [&](DivRR result, long long q) {
    CHECK(from_bt(result.q) == q);
    CHECK(from_bt(result.r) == a - q * b);
  };
  check(divmod_trunc(A, B), qt);
  check(divmod_floor(A, B), qt - (rt != 0 && direction < 0));
  check(divmod_ceil(A, B), qt + (rt != 0 && direction > 0));
  check(divmod_euclid(A, B), qt - (rt < 0 ? (b < 0 ? -1 : 1) : 0));
  check(divmod_nearest_away(A, B), qt + (twice >= denominator ? direction : 0));
  check(divmod_nearest_even(A, B),
        qt + ((twice > denominator || (twice == denominator && qt % 2 != 0)) ? direction : 0));
}
int main_div_round() {
  for (long long a = -24; a <= 24; ++a)
    for (long long b = -12; b <= 12; ++b)
      if (b)
        check_rounding(a, b);
  std::mt19937_64 rng(0xA11D1A55ULL);
  std::uniform_int_distribution<long long> dividend(-10000000000LL, 10000000000LL),
      divisor(1, 1000000);
  for (int i = 0; i < 400; ++i) {
    const auto a = dividend(rng);
    auto b = divisor(rng);
    if (rng() % 2)
      b = -b;
    check_rounding(a, b);
  }
  std::cout << "Rounding tests: exact ties and random integer oracle passed.\n";
  return 0;
}

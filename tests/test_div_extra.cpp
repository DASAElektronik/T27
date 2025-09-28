// tests/test_div_extra.cpp
//
// Standalone test program to exercise additional division paths
// and raise line/branch coverage for div.hpp/div.cpp/convert.cpp.
//
// Build as its own test executable to avoid conflicting with the
// existing test_main.cpp in t27_tests.
#include <t27/num/div_long.hpp>
#include <t27/num/trit.hpp>
#include <t27/util/conv_int.hpp>   // from_int64, to_int64, parse_bt, to_string_bt
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace t27::num;
using namespace t27::util;

static long long i64(std::span<const Trit> v) {
  return to_int64(v);
}

static void print_case(const char* tag,
                       std::span<const Trit> a,
                       std::span<const Trit> b,
                       std::span<const Trit> q,
                       std::span<const Trit> r) {
  std::cout << "[" << tag << "] "
            << "a=" << to_string_bt(a)
            << " b=" << to_string_bt(b)
            << " q=" << to_string_bt(q)
            << " r=" << to_string_bt(r)
            << "  |  "
            << i64(a) << " = " << i64(q) << " * " << i64(b) << " + " << i64(r)
            << "\n";
}

static void check_case(const char* tag, std::vector<Trit> a, std::vector<Trit> b) {
  // ensure b != 0
  if (b.empty()) b.push_back(Trit::P);

  auto [q, r] = divmod_long(a, b);

  // Invariants in int64 domain (safe for our sizes here)
  long long ia = i64(a);
  long long ib = i64(b);
  long long iq = i64(q);
  long long ir = i64(r);

  // 1) Reconstruction
  assert(iq * ib + ir == ia);

  // 2) Remainder magnitude
  assert(std::llabs(ir) < std::llabs(ib) || ib == 0); // second part defensive

  // 3) Sign sanity: if a==0 then r==0 and q==0
  if (ia == 0) {
    assert(ir == 0);
    assert(iq == 0);
  }

  print_case(tag, a, b, q, r);
}

// produce a random balanced-trit vector of length in [minL, maxL]
static std::vector<Trit> rand_trits(std::mt19937_64& rng, int minL, int maxL, bool allow_zero = true) {
  std::uniform_int_distribution<int> len_d(minL, maxL);
  std::uniform_int_distribution<int> tri_d(0, 2);
  int L = len_d(rng);
  std::vector<Trit> v;
  v.reserve(L);
  for (int i=0; i<L; ++i) {
    int t = tri_d(rng);
    v.push_back(t==0? Trit::N : (t==1? Trit::Z : Trit::P));
  }
  // canonicalize: strip trailing zeros
  while (!v.empty() && v.back()==Trit::Z) v.pop_back();
  if (!allow_zero && v.empty()) v.push_back(Trit::P);
  return v;
}

int main() try {
  std::cout << "=== div extra tests ===\n";

  // 1) divide by +1 / -1
  check_case("unit+1", from_int64(123456), parse_bt("+"));
  check_case("unit-1", from_int64(-9876),  parse_bt("-"));

  // 2) power-of-3 divisors (shift paths)
  // 3^0, 3^3, 3^7
  {
    std::vector<Trit> b0 = {Trit::P};
    std::vector<Trit> b3(3, Trit::Z); b3.push_back(Trit::P);
    std::vector<Trit> b7(7, Trit::Z); b7.push_back(Trit::P);
    check_case("pow3-0", parse_bt("+0-+"), b0);
    check_case("pow3-3", parse_bt("+0-+00"), b3);
    check_case("pow3-7", from_int64(1234567), b7);
  }

  // 3) |a| < |b| -> q=0, r=a
  {
    auto a = parse_bt("+0");     // 1 + 3
    auto b = parse_bt("+00");    // 1 + 3^2  (|b| > |a|)
    auto [q,r] = divmod_long(a,b);
    assert(i64(q) == 0 && i64(r) == i64(a));
    print_case("short-a", a,b,q,r);
  }

  // 4) similar length, "tight" case
  check_case("tight", parse_bt("+0-+"), parse_bt("+00-"));

  // 5) sign mix
  check_case("mix-1", from_int64(-42), from_int64(+5));
  check_case("mix-2", from_int64(+42), from_int64(-5));

  // 6) random families
  std::mt19937_64 rng(0xC0FFEE);
  for (int t=0; t<100; ++t) {
    auto a = rand_trits(rng, 1, 30);
    auto b = rand_trits(rng, 1, 30, /*allow_zero=*/false);
    check_case("rnd", a, b);
  }

  std::cout << "div extra tests passed.\n";
  return 0;
}
catch (const std::exception& e) {
  std::cerr << "Exception: " << e.what() << "\n";
  return 2;
}
catch (...) {
  std::cerr << "Unknown exception\n";
  return 3;
}
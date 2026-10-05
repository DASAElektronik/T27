// SPDX-License-Identifier: Apache-2.0
#include "t27/num/convert.hpp"
#include "t27/num/div3k.hpp"
#include "t27/num/div_long_strict.hpp"
#include <iostream>
#include <sstream>
#include <string>
using namespace t27::num;
int main() {
  std::string line;
  while (std::getline(std::cin, line)) {
    try {
      std::istringstream input(line);
      std::string op, x, y;
      input >> op >> x >> y;
      if (op == "int") {
        std::cout << from_bt(parse_bt(x)) << '\n';
        continue;
      }
      auto a = parse_bt(x);
      if (op == "shl" || op == "shr" || op == "d3") {
        const int k = std::stoi(y);
        if (op == "d3") {
          auto d = divmod3k_balanced(a, k);
          std::cout << to_string(d.q) << ' ' << to_string(d.r) << '\n';
        } else
          std::cout << to_string(op == "shl" ? shl(a, k) : shr(a, k)) << '\n';
        continue;
      }
      if (op == "wshl" || op == "wshr") {
        auto result =
            op == "wshl" ? shl27(to_word27(a), std::stoi(y)) : shr27(to_word27(a), std::stoi(y));
        std::cout << result.first.to_string() << ' ' << result.second.overflow << ' '
                  << result.second.inexact << '\n';
        continue;
      }
      auto b = parse_bt(y);
      if (op == "add" || op == "sub" || op == "mul") {
        auto value = op == "add" ? add(a, b) : op == "sub" ? sub(a, b) : mul(a, b);
        std::cout << to_string(value) << '\n';
        continue;
      }
      if (op == "wadd" || op == "wsub" || op == "wmul") {
        auto A = to_word27(a), B = to_word27(b);
        auto result = op == "wadd" ? add27(A, B) : op == "wsub" ? sub27(A, B) : mul27(A, B);
        std::cout << result.first.to_string() << ' ' << result.second.overflow << ' '
                  << result.second.inexact << '\n';
        continue;
      }
      if (op == "cmp") {
        std::cout << int(cmp(a, b)) << '\n';
        continue;
      }
      if (op == "div") {
        auto d = divmod_long_strict(a, b);
        std::cout << to_string(d.q) << ' ' << to_string(d.r) << '\n';
        continue;
      }
      if (op == "wdiv") {
        auto d = divmod27(to_word27(a), to_word27(b));
        std::cout << d.q.to_string() << ' ' << d.r.to_string() << ' ' << d.divide_by_zero << ' '
                  << d.overflow_q << ' ' << d.inexact << '\n';
        continue;
      }
      if (op == "wtrunc" || op == "wfloor" || op == "wceil" || op == "weuclid" || op == "waway" ||
          op == "weven") {
        using WFn = DivRR27 (*)(const Tword27 &, const Tword27 &);
        WFn f = op == "wtrunc"    ? divmod_trunc27
                : op == "wfloor"  ? divmod_floor27
                : op == "wceil"   ? divmod_ceil27
                : op == "weuclid" ? divmod_euclid27
                : op == "waway"   ? divmod_nearest_away27
                                  : divmod_nearest_even27;
        auto d = f(to_word27(a), to_word27(b));
        std::cout << d.q.to_string() << ' ' << d.r.to_string() << ' ' << d.divide_by_zero << ' '
                  << d.overflow_q << ' ' << d.inexact << '\n';
        continue;
      }
      using Fn = DivRR (*)(std::span<const Trit>, std::span<const Trit>);
      Fn f = op == "trunc"    ? divmod_trunc
             : op == "floor"  ? divmod_floor
             : op == "ceil"   ? divmod_ceil
             : op == "euclid" ? divmod_euclid
             : op == "away"   ? divmod_nearest_away
             : op == "even"   ? divmod_nearest_even
                              : nullptr;
      if (!f)
        throw std::invalid_argument("unknown oracle operation");
      auto d = f(a, b);
      std::cout << to_string(d.q) << ' ' << to_string(d.r) << '\n';
    } catch (const std::overflow_error &) {
      std::cout << "OVERFLOW\n";
    } catch (const std::invalid_argument &) {
      std::cout << "INVALID\n";
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
      return 1;
    }
  }
}

// SPDX-License-Identifier: Apache-2.0
#include <iostream>
#include <t27/num/convert.hpp>
#include <t27/num/div.hpp>
int main() {
  using namespace t27::num;
  auto result = divmod(to_bt(5), to_bt(4));
  std::cout << from_bt(result.q) << " remainder " << from_bt(result.r) << '\n';
  return from_bt(result.q) == 1 && from_bt(result.r) == 1 ? 0 : 1;
}

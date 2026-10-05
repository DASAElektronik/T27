<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Compilable example

The following is built and tested as `t27_example` by CMake.

```cpp
#include <t27/num/convert.hpp>
#include <t27/num/div.hpp>
#include <iostream>
int main() {
  using namespace t27::num;
  auto result = divmod(to_bt(5), to_bt(4));
  std::cout << from_bt(result.q) << " remainder " << from_bt(result.r) << '\n';
}
```

Ordinary text is MSB-first: `parse_bt("+0")` is 3. Use explicit
`util::parse_bt_lsb("+0")` only for a legacy least-significant-first string (value 1).
Link the library target `T27::t27`, including when using utility converters.

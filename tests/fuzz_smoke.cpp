// SPDX-License-Identifier: Apache-2.0
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *, std::size_t);
int main() {
  const std::uint8_t cycle[] = {0, 0, 2, 2, 2, 1};
  LLVMFuzzerTestOneInput(cycle, sizeof(cycle));
  std::mt19937_64 rng(0x2720F00D);
  for (int i = 0; i < 1000; ++i) {
    std::vector<std::uint8_t> data(static_cast<std::size_t>(rng() % 257));
    for (auto &value : data)
      value = static_cast<std::uint8_t>(rng());
    LLVMFuzzerTestOneInput(data.data(), data.size());
  }
  std::cout << "Deterministic fuzz harness: 1001 inputs passed.\n";
}

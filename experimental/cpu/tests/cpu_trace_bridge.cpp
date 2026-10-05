// SPDX-License-Identifier: Apache-2.0
// Test-only numeric protocol. No assembler/codec or core conversion helpers are used.
#include "t27/experimental/cpu.hpp"
#include <iostream>
#include <stdexcept>
using namespace t27::experimental;
namespace {
std::int64_t read_int() {
  std::int64_t n = 0;
  if (!(std::cin >> n))
    throw std::runtime_error("incomplete trace input");
  return n;
}
std::size_t bounded(std::int64_t n, std::int64_t maximum) {
  if (n < 0 || n > maximum)
    throw std::runtime_error("trace input bound");
  return static_cast<std::size_t>(n);
}
t27::num::Tword27 read_word() {
  auto n = read_int();
  if (n < -word_limit || n > word_limit)
    throw std::runtime_error("trace word range");
  t27::num::Tword27 w{};
  for (auto &t : w.t) {
    auto r = n % 3;
    n /= 3;
    if (r == 2) {
      r = -1;
      ++n;
    }
    if (r == -2) {
      r = 1;
      --n;
    }
    t = static_cast<t27::num::Trit>(r);
  }
  return w;
}
std::int64_t value(const t27::num::Tword27 &w) {
  std::int64_t n = 0;
  for (std::size_t k = 27; k > 0; --k)
    n = 3 * n + static_cast<int>(w.t[k - 1]);
  return n;
}
const char *reason(Stop s) {
  switch (s) {
  case Stop::running:
    return "running";
  case Stop::halted:
    return "halted";
  case Stop::fault:
    return "fault";
  case Stop::step_limit:
    return "step_limit";
  }
  throw std::runtime_error("unknown stop");
}
const char *fault(Fault f) {
  switch (f) {
  case Fault::none:
    return "none";
  case Fault::fetch_address:
    return "fetch_address";
  case Fault::illegal_instruction:
    return "illegal_instruction";
  case Fault::divide_by_zero:
    return "divide_by_zero";
  case Fault::data_address:
    return "data_address";
  case Fault::branch_address:
    return "branch_address";
  case Fault::stack_overflow:
    return "stack_overflow";
  case Fault::stack_underflow:
    return "stack_underflow";
  }
  throw std::runtime_error("unknown fault");
}
void snapshot(const Machine &m, RunResult r) {
  const auto &s = m.state();
  std::cout << "{\"reason\":\"" << reason(r.reason) << "\",\"retired\":" << r.retired
            << ",\"pc\":" << s.pc << ",\"sp\":" << s.sp
            << ",\"halted\":" << (s.halted ? "true" : "false") << ",\"fault\":\"" << fault(s.fault)
            << "\",\"flags\":[" << static_cast<int>(s.flags.sign) << ','
            << (s.flags.overflow ? "true" : "false") << ',' << (s.flags.inexact ? "true" : "false")
            << "],\"registers\":[";
  for (std::size_t i = 0; i < register_count; ++i) {
    if (i)
      std::cout << ',';
    std::cout << value(s.registers[i]);
  }
  std::cout << "],\"memory\":[";
  for (std::size_t i = 0; i < m.memory().size(); ++i) {
    if (i)
      std::cout << ',';
    std::cout << value(m.memory()[i]);
  }
  std::cout << "]}\n";
}
} // namespace
int main() {
  try {
    const auto count = bounded(read_int(), 100000);
    for (std::size_t c = 0; c < count; ++c) {
      const auto size = bounded(read_int(), 4096), image_size = bounded(read_int(), 4096);
      const auto entry = bounded(read_int(), 4096), begin = bounded(read_int(), 4096),
                 end = bounded(read_int(), 4096);
      const auto runs = bounded(read_int(), 1000);
      std::array<t27::num::Tword27, register_count> registers{};
      for (auto &w : registers)
        w = read_word();
      std::vector<t27::num::Tword27> image(image_size);
      for (auto &w : image)
        w = read_word();
      Machine m(image, size);
      m.configure_stack(begin, end);
      m.reset(entry);
      for (std::size_t i = 0; i < register_count; ++i)
        m.set_register(i, registers[i]);
      snapshot(m, {Stop::running, 0});
      for (std::size_t i = 0; i < runs; ++i)
        snapshot(m, m.run(bounded(read_int(), 1000)));
    }
    std::cin >> std::ws;
    if (!std::cin.eof())
      throw std::runtime_error("extra trace input");
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 2;
  }
}

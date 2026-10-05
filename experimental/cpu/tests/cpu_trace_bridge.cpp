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
  case Stop::input_wait:
    return "input_wait";
  case Stop::output_wait:
    return "output_wait";
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
  case Fault::io_endpoint:
    return "io_endpoint";
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
void values(std::span<const t27::num::Tword27> words) {
  std::cout << '[';
  for (std::size_t n = 0; n < words.size(); ++n) {
    if (n)
      std::cout << ',';
    std::cout << value(words[n]);
  }
  std::cout << ']';
}
void snapshot(const Machine &m, RunResult r, std::span<const t27::num::Tword27> host = {}) {
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
  std::cout << "],\"io\":{\"input\":";
  values(m.io().input);
  std::cout << ",\"output\":";
  values(m.io().output);
  std::cout << ",\"closed\":" << (m.io().input_closed ? "true" : "false")
            << ",\"input_capacity\":" << m.io_config().input_capacity
            << ",\"output_capacity\":" << m.io_config().output_capacity << "},\"host_result\":";
  values(host);
  std::cout << "}\n";
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
      const auto input_capacity = bounded(read_int(), 4096),
                 output_capacity = bounded(read_int(), 4096);
      const bool closed = bounded(read_int(), 1) != 0;
      std::vector<t27::num::Tword27> incoming(bounded(read_int(), 4096));
      for (auto &w : incoming)
        w = read_word();
      Machine m(image, size, {input_capacity, output_capacity});
      if (!m.feed_input(incoming))
        throw std::runtime_error("initial input capacity");
      if (closed)
        m.close_input();
      m.configure_stack(begin, end);
      m.reset(entry);
      for (std::size_t i = 0; i < register_count; ++i)
        m.set_register(i, registers[i]);
      snapshot(m, {Stop::running, 0});
      for (std::size_t i = 0; i < runs; ++i) {
        const auto action = bounded(read_int(), 5);
        if (action == 0) {
          snapshot(m, m.run(bounded(read_int(), 1000)));
        } else if (action == 1) {
          std::vector<t27::num::Tword27> words(bounded(read_int(), 4096));
          for (auto &w : words)
            w = read_word();
          t27::num::Tword27 accepted{};
          if (m.feed_input(words))
            accepted.t[0] = t27::num::Trit::P;
          snapshot(m, {Stop::running, 0}, std::span(&accepted, 1));
        } else if (action == 2) {
          m.close_input();
          snapshot(m, {Stop::running, 0});
        } else if (action == 3) {
          const auto drained = m.drain_output();
          snapshot(m, {Stop::running, 0}, drained);
        } else if (action == 4) {
          m.reset(bounded(read_int(), 4096));
          snapshot(m, {Stop::running, 0});
        } else {
          m.reset_io();
          snapshot(m, {Stop::running, 0});
        }
      }
    }
    std::cin >> std::ws;
    if (!std::cin.eof())
      throw std::runtime_error("extra trace input");
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 2;
  }
}

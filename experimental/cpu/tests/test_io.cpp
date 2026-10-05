// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/assembler.hpp"
#include "t27/experimental/cpu.hpp"
#include "t27/num/convert.hpp"
#include "t27/num/fixed.hpp"
#include "test_support.hpp"
#include <iostream>
#include <string>
#include <utility>
using namespace t27::experimental;
using namespace t27::num;
static Tword27 word(std::int64_t n) {
  return to_word27(to_bt(n));
}
static std::int64_t reg(const Machine &m, std::size_t n) {
  return from_bt(m.state().registers[n].span());
}
static void unchanged_stop(Machine &m, Stop stop, Fault fault = Fault::none) {
  const auto s = m.state();
  const auto memory = std::vector<Tword27>(m.memory().begin(), m.memory().end());
  const auto io = m.io();
  const auto r = m.run(9);
  CHECK(r.reason == stop && r.retired == 0);
  CHECK(m.state().pc == s.pc && m.state().sp == s.sp && m.state().flags == s.flags);
  CHECK(m.state().halted == s.halted && m.state().fault == fault);
  for (std::size_t n = 0; n < 9; ++n)
    CHECK(m.state().registers[n].t == s.registers[n].t);
  for (std::size_t n = 0; n < memory.size(); ++n)
    CHECK(m.memory()[n].t == memory[n].t);
  CHECK(m.io().input_closed == io.input_closed);
  CHECK(m.io().input.size() == io.input.size() && m.io().output.size() == io.output.size());
  for (std::size_t n = 0; n < io.input.size(); ++n)
    CHECK(m.io().input[n].t == io.input[n].t);
  for (std::size_t n = 0; n < io.output.size(); ++n)
    CHECK(m.io().output[n].t == io.output[n].t);
}
static void encoding() {
  CHECK(from_bt(assemble("IN R0,R1,0").words[0].span()) == -844);
  CHECK(from_bt(assemble("OUT R8,0").words[0].span()) == 964);
  for (std::size_t d = 0; d < 9; ++d) {
    for (std::size_t s = 0; s < 9; ++s) {
      if (d == s) {
        check_throws<std::invalid_argument>([&] { (void)encode({Opcode::input, d, s}); });
        continue;
      }
      for (auto endpoint : {-immediate_limit, std::int64_t{0}, immediate_limit}) {
        const Instruction i{Opcode::input, d, s, 0, endpoint};
        CHECK(decode(encode(i)) == i);
      }
    }
    const Instruction i{Opcode::output, 0, d};
    CHECK(decode(encode(i)) == i);
  }
  check_throws<std::invalid_argument>([] { (void)decode(word(-1087)); }); // IN R0,R0,0
  for (auto source : {"IN R0,R1", "IN R0,R0,0", "IN R9,R0,0", "IN R0,R1,193710245", "OUT R0",
                      "OUT R0,0,1", "IN R0,R1,label\nlabel: HALT"})
    check_throws<AssemblyError>([&] { (void)assemble(source); });
}
static void queues() {
  Machine m(assemble("IN R0,R1,0\nOUT R0,0\nJMP -3").words, 16, {2, 1});
  m.set_register(0, word(77));
  m.set_register(1, word(-4));
  unchanged_stop(m, Stop::input_wait);
  CHECK(m.run(0).reason == Stop::step_limit);
  const std::vector<Tword27> input{word(0), word(-word_limit)};
  CHECK(m.feed_input(input));
  CHECK(!m.feed_input(input)); // Whole batch rejected, no partial append.
  CHECK(m.run(4).retired == 4 && reg(m, 0) == -word_limit && reg(m, 1) == 1);
  unchanged_stop(m, Stop::output_wait);
  CHECK(m.io().output.size() == 1 && from_bt(m.io().output[0].span()) == 0);
  auto drained = m.drain_output();
  CHECK(drained.size() == 1 && from_bt(drained[0].span()) == 0);
  CHECK(m.run(2).retired == 2); // OUT then JMP, no duplicate output on retry.
  unchanged_stop(m, Stop::input_wait);
  m.close_input();
  m.close_input();
  CHECK(!m.feed_input(input));
  CHECK(m.run(1).retired == 1 && reg(m, 1) == 0 && reg(m, 0) == -word_limit);
  unchanged_stop(m, Stop::output_wait);
  m.reset(); // I/O retained, CPU cleared.
  CHECK(m.io().input_closed && m.io().output.size() == 1);
  CHECK(m.run(1).retired == 1 && reg(m, 1) == 0 && reg(m, 0) == 0);
  m.reset_io();
  CHECK(!m.io().input_closed && m.io().output.empty());
  CHECK(m.state().pc == 1); // Independent lifecycle.
  m.reset();
  unchanged_stop(m, Stop::input_wait);
  CHECK(m.feed_input(std::span(input).first(1)));
  CHECK(m.feed_input(m.io().input)); // Overlapping host span is safely copied.
  CHECK(m.io().input.size() == 2);
  m.close_input();
  CHECK(m.run(1).retired == 1 && reg(m, 1) == 1); // Buffered words precede EOF.
  auto bad = word(1);
  bad.t[9] = static_cast<Trit>(2);
  check_throws<std::invalid_argument>([&] { (void)m.feed_input(std::span(&bad, 1)); });
  CHECK(m.io().input.size() == 1);
  Machine zero(assemble("IN R0,R1,0\nOUT R0,0").words, 2, {0, 0});
  CHECK(!zero.feed_input(input));
  unchanged_stop(zero, Stop::input_wait);
  zero.close_input();
  CHECK(zero.run(1).retired == 1);
  unchanged_stop(zero, Stop::output_wait);
  check_throws<std::invalid_argument>([&] { Machine too_large({}, 1, {1048577, 0}); });
}
static void precise_io() {
  for (auto instruction : {"IN R0,R1,1", "OUT R0,-1"}) {
    Machine m(assemble(instruction).words, 1, {0, 0});
    unchanged_stop(m, Stop::fault, Fault::io_endpoint); // Endpoint before readiness.
    m.reset_io();
    unchanged_stop(m, Stop::fault, Fault::io_endpoint);
    CHECK(m.run(0).reason == Stop::fault);
  }
  for (auto arithmetic : {"ADD R2,R3,R4", "DIV R2,R3,R4"}) {
    Machine m(assemble(std::string(arithmetic) + "\nIN R0,R1,0\nOUT R0,0\nHALT").words);
    m.set_register(3, word(arithmetic[0] == 'A' ? word_limit : -7));
    m.set_register(4, word(arithmetic[0] == 'A' ? 1 : 3));
    CHECK(m.step() == Stop::running);
    const auto f = m.state().flags;
    unchanged_stop(m, Stop::input_wait);
    const auto input = word(word_limit);
    CHECK(m.feed_input(std::span(&input, 1)));
    m.close_input();
    CHECK(m.run(3).reason == Stop::halted && m.state().flags == f);
    CHECK(reg(m, 0) == word_limit && reg(m, 1) == 1);
    m.reset(1);
    m.set_register(3, word(123));
    CHECK(m.run(1).retired == 1 && reg(m, 1) == 0 && reg(m, 3) == 123);
  }
  Machine last(assemble("OUT R0,0").words, 1);
  CHECK(last.run(2).retired == 1 && last.state().fault == Fault::fetch_address);
  CHECK(last.io().output.size() == 1);
  Machine halt(assemble("HALT").words);
  CHECK(halt.run(1).reason == Stop::halted);
  halt.reset_io();
  CHECK(halt.run(0).reason == Stop::halted);
}
int main() {
  encoding();
  queues();
  precise_io();
  std::cout << "ISA v0.2: I/O encoding, bounds, EOF, waits, retry, reset and atomicity PASS\n";
}

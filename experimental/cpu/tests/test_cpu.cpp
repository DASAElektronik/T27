// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/cpu.hpp"
#include "t27/num/convert.hpp"
#include "t27/num/fixed.hpp"
#include "test_support.hpp"
#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <tuple>
#include <utility>
using namespace t27::experimental;
using namespace t27::num;
static Tword27 word(std::int64_t n) {
  return to_word27(to_bt(n));
}
static std::int64_t number(const Tword27 &w) {
  return from_bt(w.span());
}
static std::vector<Tword27> program(std::initializer_list<Instruction> code) {
  std::vector<Tword27> out;
  for (auto i : code)
    out.push_back(encode(i));
  return out;
}
static bool same_registers(const State &a, const State &b) {
  for (std::size_t i = 0; i < register_count; ++i)
    if (a.registers[i].t != b.registers[i].t)
      return false;
  return true;
}
static void codec() {
  // External arithmetic vectors fix field order and register bias independently of round trips.
  for (auto [i, value] :
       std::vector<std::pair<Instruction, std::int64_t>>{{{Opcode::nop}, 0},
                                                         {{Opcode::halt}, 1},
                                                         {{Opcode::li, 0, 0, 0, 5}, 98309},
                                                         {{Opcode::mov, 8, 0}, -861},
                                                         {{Opcode::add, 4, 8, 0}, -7772},
                                                         {{Opcode::store, 0, 1, 8, -7}, -129752},
                                                         {{Opcode::jmp, 0, 0, 0, -1}, -19672}}) {
    CHECK(encode(i).t == word(value).t);
    CHECK(decode(word(value)) == i);
  }
  for (std::size_t d = 0; d < 9; ++d) {
    for (auto imm : {-immediate_limit, std::int64_t{0}, immediate_limit}) {
      const Instruction i{Opcode::li, d, 0, 0, imm};
      CHECK(decode(encode(i)) == i);
    }
    for (std::size_t a = 0; a < 9; ++a) {
      for (std::size_t b = 0; b < 9; ++b) {
        for (auto op : {Opcode::add, Opcode::sub, Opcode::mul, Opcode::div, Opcode::rem}) {
          const Instruction i{op, d, a, b};
          CHECK(decode(encode(i)) == i);
        }
      }
    }
  }
  for (int code = -13; code < -8; ++code)
    check_throws<std::invalid_argument>([&] { (void)decode(word(code)); });
  check_throws<std::invalid_argument>([] { (void)encode({static_cast<Opcode>(14)}); });
  check_throws<std::invalid_argument>([] { (void)encode({Opcode::li, 9}); });
  check_throws<std::invalid_argument>([] { (void)encode({Opcode::nop, 1}); });
  check_throws<std::invalid_argument>([] { (void)encode({Opcode::mov, 0, 0, 1}); });
  check_throws<std::invalid_argument>(
      [] { (void)encode({Opcode::li, 0, 0, 0, immediate_limit + 1}); });
  check_throws<std::invalid_argument>([] { (void)encode({Opcode::nop, 0, 0, 0, 1}); });
  for (std::size_t offset = 3; offset < 27; ++offset) {
    auto bad = encode({Opcode::halt});
    bad.t[offset] = Trit::P;
    check_throws<std::invalid_argument>([&] { (void)decode(bad); });
  }
  for (std::size_t offset = 0; offset < 27; ++offset) {
    auto bad = Tword27{};
    bad.t[offset] = static_cast<Trit>(2);
    check_throws<std::invalid_argument>([&] { (void)decode(bad); });
  }
}
static void arithmetic() {
  for (auto op : {Opcode::add, Opcode::sub, Opcode::mul, Opcode::div, Opcode::rem}) {
    Machine m(program({{op, 0, 0, 1}, {Opcode::halt}}));
    for (std::int64_t a = -20; a <= 20; ++a) {
      for (std::int64_t b = -20; b <= 20; ++b) {
        if (b == 0 && (op == Opcode::div || op == Opcode::rem))
          continue;
        m.reset();
        m.set_register(0, word(a));
        m.set_register(1, word(b));
        std::int64_t expected = 0;
        switch (op) {
        case Opcode::add:
          expected = a + b;
          break;
        case Opcode::sub:
          expected = a - b;
          break;
        case Opcode::mul:
          expected = a * b;
          break;
        case Opcode::div:
          expected = a / b;
          break;
        case Opcode::rem:
          expected = a % b;
          break;
        default:
          CHECK(false);
        }
        CHECK(m.step() == Stop::running);
        CHECK(number(m.state().registers[0]) == expected);
        CHECK(!m.state().flags.overflow);
        CHECK(m.state().flags.inexact == (op == Opcode::div && a % b != 0));
        CHECK(static_cast<int>(m.state().flags.sign) == ((expected > 0) - (expected < 0)));
      }
    }
  }
  for (auto [op, a, b, expected, overflow] :
       std::vector<std::tuple<Opcode, std::int64_t, std::int64_t, std::int64_t, bool>>{
           {Opcode::add, word_limit, 1, -word_limit, true},
           {Opcode::sub, -word_limit, 1, word_limit, true},
           {Opcode::mul, word_limit, 2, -1, true},
           {Opcode::div, -word_limit, -1, word_limit, false}}) {
    Machine m(program({{op, 0, 0, 1}}));
    m.set_register(0, word(a));
    m.set_register(1, word(b));
    CHECK(m.step() == Stop::running);
    CHECK(number(m.state().registers[0]) == expected);
    CHECK(m.state().flags.overflow == overflow);
  }
}
static void programs_and_control() {
  Machine m(program({{Opcode::li, 0, 0, 0, 0},
                     {Opcode::li, 1, 0, 0, 1},
                     {Opcode::li, 2, 0, 0, 11},
                     {Opcode::li, 3, 0, 0, 1},
                     {Opcode::add, 0, 0, 1},
                     {Opcode::add, 1, 1, 3},
                     {Opcode::sub, 4, 1, 2},
                     {Opcode::jnz, 0, 4, 0, -4},
                     {Opcode::store, 0, 8, 0, 40},
                     {Opcode::load, 5, 8, 0, 40},
                     {Opcode::mov, 6, 5},
                     {Opcode::halt}}));
  CHECK(m.run(0).reason == Stop::step_limit);
  CHECK(m.run(8).reason == Stop::step_limit);
  const auto end = m.run(1000);
  CHECK(end.reason == Stop::halted && end.retired == 40);
  CHECK(m.state().pc == 11 && number(m.memory()[40]) == 55);
  CHECK(number(m.state().registers[5]) == 55 && number(m.state().registers[6]) == 55);
  CHECK(m.run(10).retired == 0 && m.step() == Stop::halted);
  m.reset();
  CHECK(m.state().pc == 0 && number(m.state().registers[0]) == 0);
  CHECK(number(m.memory()[40]) == 55); // Reset does not reload memory.
  Machine loop(program({{Opcode::jmp, 0, 0, 0, -1}}));
  CHECK(loop.run(17).retired == 17 && loop.state().pc == 0);
  CHECK(loop.state().fault == Fault::none && !loop.state().halted);
  for (auto op : {Opcode::jz, Opcode::jnz}) {
    for (int value : {-1, 0, 1}) {
      Machine branch(program({{op, 0, 1, 0, 1}, {Opcode::nop}, {Opcode::halt}}));
      branch.set_register(1, word(value));
      CHECK(branch.step() == Stop::running);
      const bool taken = op == Opcode::jz ? value == 0 : value != 0;
      CHECK(branch.state().pc == (taken ? 2u : 1u));
    }
  }
  Machine self_modify(program({{Opcode::store, 0, 8, 1, 1}, {Opcode::nop}}), 2);
  self_modify.set_register(1, encode({Opcode::halt}));
  CHECK(self_modify.run(4).reason == Stop::halted && self_modify.state().pc == 1);
}
static void flags_and_aliasing() {
  // Preserve nonzero flags across control flow, stores, halt and faults.
  for (auto i :
       {Instruction{Opcode::nop}, Instruction{Opcode::store, 0, 8, 0, 20}, Instruction{Opcode::jmp},
        Instruction{Opcode::jz, 0, 8}, Instruction{Opcode::jnz, 0, 0}, Instruction{Opcode::halt},
        Instruction{Opcode::div, 0, 0, 8}}) {
    Machine m(program({{Opcode::add, 0, 0, 1}, i, {Opcode::halt}}));
    m.set_register(0, word(word_limit));
    m.set_register(1, word(1));
    CHECK(m.step() == Stop::running);
    const auto flags = m.state().flags;
    CHECK(flags.sign == Trit::N && flags.overflow && !flags.inexact);
    (void)m.step();
    CHECK(m.state().flags == flags);
  }
  for (auto i : {Instruction{Opcode::li, 0, 0, 0, 7}, Instruction{Opcode::mov, 0, 2},
                 Instruction{Opcode::load, 0, 0}, Instruction{Opcode::rem, 0, 2, 3}}) {
    Machine m(program({{Opcode::div, 0, 2, 3}, i}));
    m.set_register(2, word(7));
    m.set_register(3, word(3));
    m.poke(2, word(-9));
    CHECK(m.step() == Stop::running && m.state().flags.inexact);
    CHECK(m.step() == Stop::running);
    CHECK(!m.state().flags.inexact && !m.state().flags.overflow);
    if (i.opcode == Opcode::load) {
      CHECK(number(m.state().registers[0]) == -9); // Destination aliases address base.
      CHECK(m.state().flags.sign == Trit::N);
    }
  }
  for (auto base : {-word_limit, word_limit}) {
    Machine m(program({{Opcode::load, 0, 1, 0, immediate_limit}}));
    m.set_register(1, word(base));
    CHECK(m.step() == Stop::fault && m.state().fault == Fault::data_address);
  }
}
static void faults() {
  for (auto [instruction, fault] : std::vector<std::pair<Instruction, Fault>>{
           {{Opcode::div, 0, 1, 2}, Fault::divide_by_zero},
           {{Opcode::rem, 0, 1, 2}, Fault::divide_by_zero},
           {{Opcode::load, 0, 2, 0, -1}, Fault::data_address},
           {{Opcode::store, 0, 2, 1, 256}, Fault::data_address},
           {{Opcode::jmp, 0, 0, 0, -2}, Fault::branch_address},
           {{Opcode::jz, 0, 2, 0, 255}, Fault::branch_address}}) {
    Machine m(program({instruction}));
    m.set_register(0, word(123));
    m.set_register(1, word(-7));
    const auto before = m.state();
    const auto old_memory = std::vector<Tword27>(m.memory().begin(), m.memory().end());
    const auto result = m.run(20);
    CHECK(result.reason == Stop::fault && result.retired == 0 && m.state().fault == fault);
    CHECK(m.state().pc == before.pc && m.state().flags == before.flags &&
          same_registers(m.state(), before));
    for (std::size_t n = 0; n < old_memory.size(); ++n)
      CHECK(old_memory[n].t == m.memory()[n].t);
    CHECK(m.step() == Stop::fault && m.run(10).retired == 0);
  }
  Machine fetch(program({{Opcode::nop}}), 1);
  CHECK(fetch.run(2).retired == 1 && fetch.state().fault == Fault::fetch_address &&
        fetch.state().pc == 1);
  Machine illegal(std::vector<Tword27>{word(-13)});
  CHECK(illegal.step() == Stop::fault && illegal.state().fault == Fault::illegal_instruction);
  illegal.poke(0, encode({Opcode::halt}));
  illegal.reset();
  CHECK(illegal.run(1).reason == Stop::halted);
  Machine skipped(program({{Opcode::jnz, 0, 0, 0, -immediate_limit}, {Opcode::halt}}));
  CHECK(skipped.run(3).reason == Stop::halted); // Untaken target is not checked.
  check_throws<std::out_of_range>([&] { skipped.reset(256); });
  check_throws<std::out_of_range>([&] { skipped.poke(256, word(0)); });
  check_throws<std::out_of_range>([&] { skipped.set_register(9, word(0)); });
  check_throws<std::invalid_argument>([] { Machine bad({}, 0); });
  check_throws<std::invalid_argument>(
      [] { Machine bad(program({{Opcode::nop}, {Opcode::halt}}), 1); });
  auto invalid = word(0);
  invalid.t[26] = static_cast<Trit>(2);
  check_throws<std::invalid_argument>([&] { skipped.poke(0, invalid); });
  check_throws<std::invalid_argument>([&] { skipped.set_register(0, invalid); });
  check_throws<std::invalid_argument>([&] { Machine bad(std::vector<Tword27>{invalid}); });
}
int main() {
  codec();
  arithmetic();
  programs_and_control();
  flags_and_aliasing();
  faults();
  std::cout << "ISA v0: external encodings, 3645 register tuples, small arithmetic, programs and "
               "atomic faults PASS\n";
}

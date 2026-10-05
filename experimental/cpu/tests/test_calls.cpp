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
static Tword27 word(std::int64_t value) {
  return to_word27(to_bt(value));
}
static std::int64_t number(const Tword27 &w) {
  return from_bt(w.span());
}
static void atomic_fault(Machine &m, Fault expected) {
  const auto before = m.state();
  const auto memory = std::vector<Tword27>(m.memory().begin(), m.memory().end());
  const auto result = m.run(1);
  CHECK(result.reason == Stop::fault && result.retired == 0);
  CHECK(m.state().fault == expected && m.state().pc == before.pc && m.state().sp == before.sp);
  CHECK(m.state().flags == before.flags && m.state().halted == before.halted);
  for (std::size_t n = 0; n < register_count; ++n)
    CHECK(m.state().registers[n].t == before.registers[n].t);
  for (std::size_t n = 0; n < memory.size(); ++n)
    CHECK(m.memory()[n].t == memory[n].t);
  CHECK(m.step() == Stop::fault && m.run(4).retired == 0);
}
static void encoding() {
  for (const auto &[source, expected] :
       std::vector<std::pair<std::string, std::int64_t>>{{"CALL 5", 98414},
                                                         {"RET", -2},
                                                         {"PUSH R0", -975},
                                                         {"POP R8", 104},
                                                         {"JMPR R8", 967},
                                                         {"CALLR R0", -978}}) {
    const auto result = assemble(source);
    CHECK(number(result.words[0]) == expected);
    CHECK(encode(decode(word(expected))).t == result.words[0].t);
  }
  for (auto op : {Opcode::push, Opcode::pop, Opcode::jmpr, Opcode::callr}) {
    for (std::size_t r = 0; r < register_count; ++r) {
      Instruction i{op};
      if (op == Opcode::pop)
        i.rd = r;
      else
        i.rs1 = r;
      CHECK(decode(encode(i)) == i);
      for (std::size_t offset = 0; offset < 27; ++offset) {
        const bool used = offset < 3 || (op == Opcode::pop ? offset >= 3 && offset < 5
                                                           : offset >= 5 && offset < 7);
        if (!used) {
          auto bad = encode(i);
          bad.t[offset] = Trit::P;
          check_throws<std::invalid_argument>([&] { (void)decode(bad); });
        }
      }
    }
  }
  for (auto imm : {-immediate_limit, std::int64_t{0}, immediate_limit}) {
    const Instruction i{Opcode::call, 0, 0, 0, imm};
    CHECK(decode(encode(i)) == i);
  }
  for (std::size_t offset = 3; offset < 27; ++offset) {
    auto bad = encode({Opcode::ret});
    bad.t[offset] = Trit::P;
    check_throws<std::invalid_argument>([&] { (void)decode(bad); });
    if (offset < 9) {
      bad = encode({Opcode::call});
      bad.t[offset] = Trit::P;
      check_throws<std::invalid_argument>([&] { (void)decode(bad); });
    }
  }
  for (auto source : {"RET R0", "CALL", "CALL missing", "CALL 193710245", "PUSH R9", "POP",
                      "CALLR 1", "JMPR R0,1"})
    check_throws<AssemblyError>([&] { (void)assemble(source); });
  const auto forward = assemble("CALL next\nHALT\nnext: RET");
  CHECK(decode(forward.words[0]).immediate == 1);
  const auto backward = assemble("start: RET\nCALL start");
  CHECK(decode(backward.words[1]).immediate == -2);
}
static void stack_and_reset() {
  Machine m(assemble("PUSH R0\nPUSH R1\nPOP R0\nPOP R1\nHALT").words, 7);
  CHECK(m.stack_region() == (StackRegion{5, 7}) && m.state().sp == 7);
  m.set_register(0, word(-word_limit));
  m.set_register(1, word(word_limit));
  CHECK(m.run(2).reason == Stop::step_limit && m.state().sp == 5);
  CHECK(number(m.memory()[6]) == -word_limit && number(m.memory()[5]) == word_limit);
  CHECK(m.run(3).reason == Stop::halted && m.state().sp == 7);
  CHECK(number(m.state().registers[0]) == word_limit &&
        number(m.state().registers[1]) == -word_limit);
  m.configure_stack(6, 7);
  m.reset();
  CHECK(m.state().sp == 7 && m.stack_region() == (StackRegion{6, 7}));
  CHECK(m.step() == Stop::running);
  atomic_fault(m, Fault::stack_overflow);
  m.reset(2);
  CHECK(m.state().sp == 7 && m.state().fault == Fault::none);
  atomic_fault(m, Fault::stack_underflow);
  m.reset();
  const auto sp = m.state().sp;
  check_throws<std::out_of_range>([&] { m.configure_stack(7, 6); });
  check_throws<std::out_of_range>([&] { m.configure_stack(0, 8); });
  check_throws<std::out_of_range>([&] { m.configure_stack(0, UINT64_MAX); });
  CHECK(m.state().sp == sp && m.stack_region() == (StackRegion{6, 7}));
  m.configure_stack(0, 0);
  atomic_fault(m, Fault::stack_overflow);
  Machine zero_based(assemble("PUSH R0\nHALT").words, 3);
  zero_based.configure_stack(0, 1);
  zero_based.set_register(0, word(17));
  CHECK(zero_based.step() == Stop::running && zero_based.state().sp == 0);
  CHECK(number(zero_based.memory()[0]) == 17); // Explicit host overlap is allowed.
}
static void control_and_flags() {
  Machine m(assemble("CALL sub\nHALT\nsub: LI R0,42\nRET").words, 5);
  CHECK(m.run(1).retired == 1 && m.state().pc == 2 && m.state().sp == 4);
  CHECK(number(m.memory()[4]) == 1);
  CHECK(m.run(3).reason == Stop::halted && m.state().sp == 5 && m.state().pc == 1);
  CHECK(number(m.state().registers[0]) == 42);
  CHECK(m.run(0).reason == Stop::halted);
  for (auto source : {"CALL 0", "CALLR R2", "JMPR R2", "PUSH R0", "POP R3", "RET"}) {
    Machine flags(
        assemble(std::string("LI R2,4\nPUSH R2\nADD R0,R0,R1\n") + source + "\nHALT").words);
    flags.set_register(0, word(word_limit));
    flags.set_register(1, word(1));
    CHECK(flags.run(3).reason == Stop::step_limit);
    const auto f = flags.state().flags;
    CHECK(f.overflow && f.sign == Trit::N);
    CHECK(flags.step() == Stop::running && flags.state().flags == f);
  }
  Machine indirect(assemble("LI R0,3\nCALLR R0\nHALT\nLI R0,42\nRET").words);
  CHECK(indirect.run(5).reason == Stop::halted && number(indirect.state().registers[0]) == 42);
  Machine jump(assemble("LI R0,3\nJMPR R0\nNOP\nHALT").words, 4);
  CHECK(jump.run(3).reason == Stop::halted && jump.state().sp == 4); // Needs no stack.
}
static void faults() {
  Machine full(assemble("CALL 0\nHALT").words, 2);
  atomic_fault(full, Fault::stack_overflow);
  Machine empty(assemble("RET").words);
  atomic_fault(empty, Fault::stack_underflow);
  Machine pop(assemble("POP R0").words);
  atomic_fault(pop, Fault::stack_underflow);
  Machine target_first(assemble("CALL -2").words, 1);
  atomic_fault(target_first, Fault::branch_address);
  Machine last(assemble("CALL -1").words, 1);
  last.configure_stack(0, 1);
  atomic_fault(last, Fault::branch_address);
  for (auto op : {"CALLR R0", "JMPR R0"}) {
    for (auto target : {-word_limit, std::int64_t{-1}, std::int64_t{8}, word_limit}) {
      Machine m(assemble(std::string(op) + "\nHALT").words, 8);
      m.set_register(0, word(target));
      atomic_fault(m, Fault::branch_address);
    }
  }
  Machine indirect_full(assemble("CALLR R0\nHALT").words, 2);
  indirect_full.set_register(0, word(1));
  atomic_fault(indirect_full, Fault::stack_overflow);
  Machine indirect_last(assemble("CALLR R0").words, 1);
  indirect_last.configure_stack(0, 1);
  atomic_fault(indirect_last, Fault::branch_address);
  for (auto target : {std::int64_t{-1}, std::int64_t{8}, word_limit}) {
    Machine m(assemble("PUSH R0\nRET").words, 8);
    m.set_register(0, word(target));
    CHECK(m.step() == Stop::running);
    atomic_fault(m, Fault::branch_address);
  }
  Machine corrupt(assemble("CALL f\nHALT\nf: RET").words, 4);
  CHECK(corrupt.step() == Stop::running);
  corrupt.poke(3, word(-1));
  atomic_fault(corrupt, Fault::branch_address);
}
static void recursion() {
  const std::string function = "\nCALL fact\nHALT\nfact: JZ R0,base\nPUSH R0\nLI R1,1\nSUB "
                               "R0,R0,R1\nCALL fact\nPOP R1\nMUL R0,R0,R1\nRET\nbase: LI R0,1\nRET";
  std::int64_t expected = 1;
  for (int n = 0; n <= 10; ++n) {
    if (n > 0)
      expected *= n;
    const auto program = assemble("LI R0," + std::to_string(n) + function);
    const auto size = program.words.size() + static_cast<std::size_t>(2 * n + 1);
    Machine m(program.words, size);
    for (std::size_t r = 5; r < 9; ++r)
      m.set_register(r, word(static_cast<std::int64_t>(r) * 17));
    const auto result = m.run(500);
    CHECK(result.reason == Stop::halted && result.retired == static_cast<std::uint64_t>(6 + 8 * n));
    CHECK(number(m.state().registers[0]) == expected && m.state().sp == size);
    for (std::size_t r = 5; r < 9; ++r)
      CHECK(number(m.state().registers[r]) == static_cast<std::int64_t>(r) * 17);
    for (std::size_t r = 0; r < program.words.size(); ++r)
      CHECK(m.memory()[r].t == program.words[r].t);
    if (n > 0) {
      Machine short_stack(program.words, size - 1);
      while (short_stack.state().sp > program.words.size())
        CHECK(short_stack.step() == Stop::running);
      // Reach the next stack operation (zero test / arithmetic can precede it).
      while (
          decode(short_stack.memory()[static_cast<std::size_t>(short_stack.state().pc)]).opcode !=
              Opcode::call &&
          decode(short_stack.memory()[static_cast<std::size_t>(short_stack.state().pc)]).opcode !=
              Opcode::push)
        CHECK(short_stack.step() == Stop::running);
      atomic_fault(short_stack, Fault::stack_overflow);
    }
  }
}
int main() {
  encoding();
  stack_and_reset();
  control_and_flags();
  faults();
  recursion();
  std::cout
      << "ISA v0.1: canonical calls, stack boundaries, atomic faults, ABI and recursion PASS\n";
}

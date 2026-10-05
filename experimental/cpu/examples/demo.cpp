// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/cpu.hpp"
#include "t27/num/convert.hpp"
#include <initializer_list>
#include <iostream>
#include <vector>
using namespace t27::experimental;
using t27::num::Tword27;
static std::vector<Tword27> assemble(std::initializer_list<Instruction> instructions) {
  std::vector<Tword27> result;
  for (const auto &i : instructions)
    result.push_back(encode(i));
  return result;
}
int main() {
  Machine sum(assemble({{Opcode::li, 0, 0, 0, 0},
                        {Opcode::li, 1, 0, 0, 1},
                        {Opcode::li, 2, 0, 0, 11},
                        {Opcode::li, 3, 0, 0, 1},
                        {Opcode::add, 0, 0, 1},
                        {Opcode::add, 1, 1, 3},
                        {Opcode::sub, 4, 1, 2},
                        {Opcode::jnz, 0, 4, 0, -4},
                        {Opcode::store, 0, 8, 0, 40},
                        {Opcode::halt}}));
  Machine factorial(assemble({{Opcode::li, 0, 0, 0, 1},
                              {Opcode::li, 1, 0, 0, 6},
                              {Opcode::li, 2, 0, 0, 1},
                              {Opcode::mul, 0, 0, 1},
                              {Opcode::sub, 1, 1, 2},
                              {Opcode::jnz, 0, 1, 0, -3},
                              {Opcode::halt}}));
  Machine division(assemble({{Opcode::li, 0, 0, 0, -7},
                             {Opcode::li, 1, 0, 0, 3},
                             {Opcode::div, 2, 0, 1},
                             {Opcode::rem, 3, 0, 1},
                             {Opcode::halt}}));
  const auto a = sum.run(1000), b = factorial.run(1000), c = division.run(1000);
  const auto read = [](const Tword27 &word) { return t27::num::from_bt(word.span()); };
  const auto total = read(sum.memory()[40]);
  const auto product = read(factorial.state().registers[0]);
  const auto q = read(division.state().registers[2]), r = read(division.state().registers[3]);
  std::cout << "T27 experimental ISA v0 (instruction-level reference)\n"
            << "sum 1..10 = " << total << " (" << a.retired << " instructions)\n"
            << "6! = " << product << " (" << b.retired << " instructions)\n"
            << "-7 / 3: quotient=" << q << ", remainder=" << r << "\n";
  return a.reason == Stop::halted && b.reason == Stop::halted && c.reason == Stop::halted &&
                 total == 55 && product == 720 && q == -2 && r == -1
             ? 0
             : 1;
}

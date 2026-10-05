// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/assembler.hpp"
#include "t27/experimental/cpu.hpp"
#include "t27/num/convert.hpp"
#include "test_support.hpp"
#include <iostream>
#include <string>
#include <utility>
using namespace t27::experimental;
static void forms() {
  for (const auto &[text, expected] : std::vector<std::pair<std::string, Instruction>>{
           {"nop", {Opcode::nop}},
           {"HALT", {Opcode::halt}},
           {"li r0,+5", {Opcode::li, 0, 0, 0, 5}},
           {"MOV R8, R0", {Opcode::mov, 8, 0}},
           {"ADD R0,R1,R2", {Opcode::add, 0, 1, 2}},
           {"SUB R3,R4,R5", {Opcode::sub, 3, 4, 5}},
           {"MUL R6,R7,R8", {Opcode::mul, 6, 7, 8}},
           {"DIV R1,R0,R2", {Opcode::div, 1, 0, 2}},
           {"REM R2,R0,R1", {Opcode::rem, 2, 0, 1}},
           {"LOAD R4,[R3]", {Opcode::load, 4, 3}},
           {"LOAD R4,[R3 + 12]", {Opcode::load, 4, 3, 0, 12}},
           {"STORE [R3 - 12],R4", {Opcode::store, 0, 3, 4, -12}},
           {"JMP -1", {Opcode::jmp, 0, 0, 0, -1}},
           {"JZ R2,7", {Opcode::jz, 0, 2, 0, 7}},
           {"JNZ R0,-8", {Opcode::jnz, 0, 0, 0, -8}}}) {
    const auto result = assemble(text);
    CHECK(result.words.size() == 1 && decode(result.words[0]) == expected);
    CHECK(result.source_lines == std::vector<std::size_t>{1});
  }
  CHECK(t27::num::from_bt(assemble("LI R0,5").words[0].span()) == 98309);
  for (auto n : {-immediate_limit, std::int64_t{0}, immediate_limit})
    CHECK(decode(assemble("LI R0," + std::to_string(n)).words[0]).immediate == n);
  for (auto n : {-word_limit, std::int64_t{0}, word_limit})
    CHECK(t27::num::from_bt(assemble(".word " + std::to_string(n)).words[0].span()) == n);
  CHECK(assemble("").words.empty());
  CHECK(assemble("# comment\r\n;comment\nlabel:\n").words.empty());
}
static void labels_and_execution() {
  const auto code = assemble("# comment\r\nstart: alias: LI R0, data\r\n"
                             "JMP end ; forward\nback: JNZ R1,start\n"
                             "data: .word -3812798742493\nend: LOAD R2,[R8+data]\nHALT\n");
  CHECK(code.words.size() == 6);
  CHECK(code.source_lines == (std::vector<std::size_t>{2, 3, 4, 5, 6, 7}));
  CHECK(decode(code.words[0]).immediate == 3);
  CHECK(decode(code.words[1]).immediate == 2);
  CHECK(decode(code.words[2]).immediate == -3);
  Machine m(code.words);
  CHECK(m.run(10).reason == Stop::halted);
  CHECK(t27::num::from_bt(m.state().registers[2].span()) == -word_limit);
  const auto cases = assemble("a: NOP\nA: NOP\nJMP a\nJMP A");
  CHECK(decode(cases.words[2]).immediate == -3);
  CHECK(decode(cases.words[3]).immediate == -3);
  // Exact program outcome also detects wrong relative label origin.
  Machine loop(assemble("LI R0,3\nLI R1,1\nagain: SUB R0,R0,R1\nJNZ R0,again\nHALT").words);
  const auto run = loop.run(20);
  CHECK(run.reason == Stop::halted && run.retired == 9);
  CHECK(t27::num::from_bt(loop.state().registers[0].span()) == 0);
}
static void errors() {
  for (auto text : {"LI R9,1",
                    "LI R00,1",
                    "LI X0,1",
                    "MOV R0",
                    "ADD R0,R1",
                    "NOP R0",
                    "HALT,",
                    "JMP missing",
                    "a:NOP\na:HALT",
                    "1:NOP",
                    ".x:NOP",
                    "LI R0 1",
                    "LI R0,1 trailing",
                    "LI R0,1.5",
                    "LI R0,0x10",
                    "LI R0,--1",
                    "LI R0,+-1",
                    "LI R0,193710245",
                    "LI R0,-193710245",
                    ".word 3812798742494",
                    ".word -3812798742494",
                    ".word 9223372036854775808",
                    ".word -9223372036854775808",
                    ".word -9223372036854775809",
                    ".word label",
                    "LOAD R0,[R1",
                    "LOAD R0,R1",
                    "LOAD R0,[R1-+2]",
                    "LOAD R0,[R1-9223372036854775808]",
                    "LOAD R0,[R1-]",
                    "STORE [R0],",
                    "LOAD R0,[R1+unknown]",
                    "JMP label+1",
                    "WHAT",
                    "@",
                    "LI R0,"})
    check_throws<AssemblyError>([&] { (void)assemble(text); });
  try {
    (void)assemble("NOP\n  LI R9,1\n");
    CHECK(false);
  } catch (const AssemblyError &error) {
    CHECK(error.line() == 2 && error.column() == 6);
    CHECK(std::string(error.what()).find("R0 through R8") != std::string::npos);
  }
  try {
    (void)assemble("NOP\nLI R0,");
    CHECK(false);
  } catch (const AssemblyError &error) {
    CHECK(error.line() == 2 && error.column() == 7);
  }
  const std::string nul("NOP\0HALT", 8);
  check_throws<AssemblyError>([&] { (void)assemble(nul); });
}
int main() {
  forms();
  labels_and_execution();
  errors();
  std::cout << "Assembler: instruction forms, labels, boundaries, diagnostics and execution PASS\n";
}

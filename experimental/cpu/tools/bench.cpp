// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/assembler.hpp"
#include "t27/experimental/cpu.hpp"
#include "t27/num/convert.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string_view>

using namespace t27::experimental;

static void measure(std::string_view name, std::string_view source, std::uint64_t instructions,
                    std::int64_t expected) {
  const auto program = assemble(source);
  Machine machine(program.words, 256);
  const auto once = [&] {
    machine.reset();
    const auto result = machine.run(1000);
    if (result.reason != Stop::halted || result.retired != instructions ||
        machine.state().sp != machine.stack_region().end ||
        t27::num::from_bt(machine.state().registers[0].span()) != expected)
      throw std::runtime_error("benchmark result mismatch");
  };
  for (int i = 0; i < 20; ++i)
    once();
  constexpr int iterations = 2000;
  for (int sample = 0; sample < 5; ++sample) {
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i)
      once();
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    std::cout << name << ',' << sample << ',' << iterations << ',' << instructions << ',' << elapsed
              << '\n';
  }
}

int main() {
  try {
    std::cout << "workload,sample,iterations,instructions_per_run,elapsed_ns\n";
    measure("sum100", R"(
LI R0, 0
LI R1, 100
LI R2, 1
loop:
ADD R0, R0, R1
SUB R1, R1, R2
JNZ R1, loop
HALT
)",
            304, 5050);
    measure("recursive_factorial6", R"(
LI R0, 6
CALL factorial
HALT
factorial:
JZ R0, base
PUSH R0
LI R1, 1
SUB R0, R0, R1
CALL factorial
POP R1
MUL R0, R0, R1
RET
base:
LI R0, 1
RET
)",
            54, 720);
    measure("division100", R"(
LI R0, -7
LI R1, 3
LI R2, 100
LI R3, 1
loop:
DIV R4, R0, R1
REM R5, R0, R1
SUB R2, R2, R3
JNZ R2, loop
ADD R0, R4, R5
HALT
)",
            406, -3);
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

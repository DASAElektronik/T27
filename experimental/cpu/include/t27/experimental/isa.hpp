// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "t27/num/tword.hpp"
#include <cstddef>
#include <cstdint>

namespace t27::experimental {
inline constexpr std::size_t register_count = 9;
inline constexpr std::int64_t immediate_limit = 193710244;  // (3^18 - 1) / 2
inline constexpr std::int64_t word_limit = 3812798742493LL; // (3^27 - 1) / 2

/// Experimental ISA v0.1; v0 codes 0..13 unchanged, -7..-13 reserved.
enum class Opcode : std::int8_t {
  callr = -6,
  jmpr = -5,
  pop = -4,
  push = -3,
  ret = -2,
  call = -1,
  nop = 0,
  halt,
  li,
  mov,
  add,
  sub,
  mul,
  div,
  rem,
  load,
  store,
  jmp,
  jz,
  jnz
};

/// Unused fields must be zero, both here and in the canonical encoded word.
struct Instruction {
  Opcode opcode{Opcode::nop};
  std::size_t rd{0}, rs1{0}, rs2{0};
  std::int64_t immediate{0};
  bool operator==(const Instruction &) const = default;
};

/// Throws invalid_argument for unsupported opcodes, invalid fields or noncanonical bits.
num::Tword27 encode(const Instruction &instruction);
Instruction decode(const num::Tword27 &word);
} // namespace t27::experimental

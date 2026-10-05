// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/isa.hpp"
#include "t27/num/convert.hpp"
#include <array>
#include <stdexcept>
#include <utility>

namespace t27::experimental {
namespace {
constexpr unsigned rd_mask = 1, rs1_mask = 2, rs2_mask = 4, imm_mask = 8;
constexpr std::array<unsigned, 14> fields{0, 0, 9, 3, 7, 7, 7, 7, 7, 11, 14, 8, 10, 10};
unsigned mask(Opcode op) {
  switch (op) {
  case Opcode::input:
    return rd_mask | rs1_mask | imm_mask;
  case Opcode::output:
    return rs1_mask | imm_mask;
  case Opcode::call:
    return imm_mask;
  case Opcode::ret:
    return 0;
  case Opcode::push:
  case Opcode::jmpr:
  case Opcode::callr:
    return rs1_mask;
  case Opcode::pop:
    return rd_mask;
  default:
    break;
  }
  const auto code = static_cast<int>(op);
  if (code < 0 || code >= static_cast<int>(fields.size()))
    throw std::invalid_argument("reserved ISA v0.2 opcode");
  return fields[static_cast<std::size_t>(code)];
}
void write_field(num::Tword27 &word, std::size_t start, std::int64_t value) {
  const auto digits = num::to_bt(value);
  for (std::size_t i = 0; i < digits.size(); ++i)
    word.t[start + i] = digits[i];
}
} // namespace

num::Tword27 encode(const Instruction &i) {
  const auto used = mask(i.opcode);
  if (i.opcode == Opcode::input && i.rd == i.rs1)
    throw std::invalid_argument("IN data and status registers must differ");
  for (const auto &[bit, index] : std::array<std::pair<unsigned, std::size_t>, 3>{
           {{rd_mask, i.rd}, {rs1_mask, i.rs1}, {rs2_mask, i.rs2}}}) {
    if (((used & bit) && index >= register_count) || (!(used & bit) && index != 0))
      throw std::invalid_argument("invalid or unused register field");
  }
  if (i.immediate < -immediate_limit || i.immediate > immediate_limit ||
      (!(used & imm_mask) && i.immediate != 0))
    throw std::invalid_argument("invalid or unused immediate field");
  num::Tword27 word{};
  write_field(word, 0, static_cast<int>(i.opcode));
  if (used & rd_mask)
    write_field(word, 3, static_cast<std::int64_t>(i.rd) - 4);
  if (used & rs1_mask)
    write_field(word, 5, static_cast<std::int64_t>(i.rs1) - 4);
  if (used & rs2_mask)
    write_field(word, 7, static_cast<std::int64_t>(i.rs2) - 4);
  if (used & imm_mask)
    write_field(word, 9, i.immediate);
  return word;
}

Instruction decode(const num::Tword27 &word) {
  // Validate every raw trit, including fields unused by the instruction.
  (void)num::from_bt(word.span());
  Instruction i{};
  i.opcode = static_cast<Opcode>(num::from_bt(word.span().subspan(0, 3)));
  const auto used = mask(i.opcode);
  if (used & rd_mask)
    i.rd = static_cast<std::size_t>(num::from_bt(word.span().subspan(3, 2)) + 4);
  if (used & rs1_mask)
    i.rs1 = static_cast<std::size_t>(num::from_bt(word.span().subspan(5, 2)) + 4);
  if (used & rs2_mask)
    i.rs2 = static_cast<std::size_t>(num::from_bt(word.span().subspan(7, 2)) + 4);
  if (used & imm_mask)
    i.immediate = num::from_bt(word.span().subspan(9, 18));
  if (encode(i).t != word.t)
    throw std::invalid_argument("nonzero reserved instruction field");
  return i;
}
} // namespace t27::experimental

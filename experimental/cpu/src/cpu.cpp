// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/cpu.hpp"
#include "t27/num/convert.hpp"
#include "t27/num/div.hpp"
#include "t27/num/fixed.hpp"
#include <algorithm>
#include <stdexcept>

namespace t27::experimental {
Machine::Machine(std::span<const num::Tword27> image, std::size_t memory_words) {
  if (memory_words == 0 || memory_words > static_cast<std::uint64_t>(word_limit) + 1 ||
      image.size() > memory_words)
    throw std::invalid_argument("invalid memory/image size");
  for (const auto &word : image)
    (void)num::from_bt(word.span());
  memory_.resize(memory_words);
  std::copy(image.begin(), image.end(), memory_.begin());
  configure_stack(image.size(), memory_.size());
}
void Machine::configure_stack(std::uint64_t begin, std::uint64_t end) {
  if (begin > end || end > memory_.size())
    throw std::out_of_range("stack region");
  stack_ = {begin, end};
  state_.sp = end;
}
void Machine::reset(std::uint64_t entry) {
  if (entry >= memory_.size())
    throw std::out_of_range("entry address");
  state_ = State{};
  state_.pc = entry;
  state_.sp = stack_.end;
}
void Machine::set_register(std::size_t index, const num::Tword27 &value) {
  if (index >= register_count)
    throw std::out_of_range("register index");
  (void)num::from_bt(value.span());
  state_.registers[index] = value;
}
void Machine::poke(std::size_t address, const num::Tword27 &value) {
  if (address >= memory_.size())
    throw std::out_of_range("memory address");
  (void)num::from_bt(value.span());
  memory_[address] = value;
}
Stop Machine::step() {
  if (state_.fault != Fault::none)
    return Stop::fault;
  if (state_.halted)
    return Stop::halted;
  const auto trap = [this](Fault fault) {
    state_.fault = fault;
    return Stop::fault;
  };
  if (state_.pc >= memory_.size())
    return trap(Fault::fetch_address);
  Instruction i{};
  try {
    i = decode(memory_[static_cast<std::size_t>(state_.pc)]);
  } catch (const std::invalid_argument &) {
    return trap(Fault::illegal_instruction);
  }

  // Stage all architectural changes. A fault only updates the fault latch.
  auto next = state_;
  next.pc = state_.pc + 1;
  const auto &a = state_.registers[i.rs1];
  const auto &b = state_.registers[i.rs2];
  const auto write = [&](const num::Tword27 &value, bool overflow = false, bool inexact = false) {
    next.registers[i.rd] = value;
    const auto signed_value = num::from_bt(value.span());
    next.flags = {signed_value < 0   ? num::Trit::N
                  : signed_value > 0 ? num::Trit::P
                                     : num::Trit::Z,
                  overflow, inexact};
  };
  const auto in_memory = [this](std::int64_t address) {
    return address >= 0 && static_cast<std::uint64_t>(address) < memory_.size();
  };
  bool store = false;
  std::size_t store_address = 0;
  num::Tword27 store_value{};
  switch (i.opcode) {
  case Opcode::nop:
    break;
  case Opcode::halt:
    next.halted = true;
    next.pc = state_.pc;
    break;
  case Opcode::li:
    write(num::to_word27(num::to_bt(i.immediate)));
    break;
  case Opcode::mov:
    write(a);
    break;
  case Opcode::add: {
    auto [v, f] = num::add27(a, b);
    write(v, f.overflow);
    break;
  }
  case Opcode::sub: {
    auto [v, f] = num::sub27(a, b);
    write(v, f.overflow);
    break;
  }
  case Opcode::mul: {
    auto [v, f] = num::mul27(a, b);
    write(v, f.overflow);
    break;
  }
  case Opcode::div:
  case Opcode::rem: {
    const auto result = num::divmod27(a, b);
    if (result.divide_by_zero)
      return trap(Fault::divide_by_zero);
    if (i.opcode == Opcode::div)
      write(result.q, result.overflow_q, result.inexact);
    else
      write(result.r);
    break;
  }
  case Opcode::load:
  case Opcode::store: {
    // A 27-trit value plus an 18-trit displacement fits int64 exactly.
    const auto address = num::from_bt(a.span()) + i.immediate;
    if (!in_memory(address))
      return trap(Fault::data_address);
    const auto index = static_cast<std::size_t>(address);
    if (i.opcode == Opcode::load)
      write(memory_[index]);
    else {
      store = true;
      store_address = index;
      store_value = b;
    }
    break;
  }
  case Opcode::call:
  case Opcode::callr:
  case Opcode::jmpr: {
    const auto target = i.opcode == Opcode::call
                            ? static_cast<std::int64_t>(state_.pc) + 1 + i.immediate
                            : num::from_bt(a.span());
    // Calls require an executable return address before modifying the stack.
    if (!in_memory(target) || (i.opcode != Opcode::jmpr && next.pc >= memory_.size()))
      return trap(Fault::branch_address);
    if (i.opcode != Opcode::jmpr) {
      if (state_.sp == stack_.begin)
        return trap(Fault::stack_overflow);
      store_value = num::to_word27(num::to_bt(static_cast<std::int64_t>(next.pc)));
      next.sp = state_.sp - 1;
      store_address = static_cast<std::size_t>(next.sp);
      store = true;
    }
    next.pc = static_cast<std::uint64_t>(target);
    break;
  }
  case Opcode::push:
    if (state_.sp == stack_.begin)
      return trap(Fault::stack_overflow);
    next.sp = state_.sp - 1;
    store_address = static_cast<std::size_t>(next.sp);
    store_value = a;
    store = true;
    break;
  case Opcode::pop:
  case Opcode::ret: {
    if (state_.sp == stack_.end)
      return trap(Fault::stack_underflow);
    const auto &value = memory_[static_cast<std::size_t>(state_.sp)];
    if (i.opcode == Opcode::ret) {
      const auto target = num::from_bt(value.span());
      if (!in_memory(target))
        return trap(Fault::branch_address);
      next.pc = static_cast<std::uint64_t>(target);
    } else {
      next.registers[i.rd] = value; // Restore without changing arithmetic flags.
    }
    next.sp = state_.sp + 1;
    break;
  }
  case Opcode::jmp:
  case Opcode::jz:
  case Opcode::jnz: {
    const bool zero = num::from_bt(a.span()) == 0;
    const bool taken = i.opcode == Opcode::jmp || (i.opcode == Opcode::jz ? zero : !zero);
    if (taken) {
      const auto target = static_cast<std::int64_t>(state_.pc) + 1 + i.immediate;
      if (!in_memory(target))
        return trap(Fault::branch_address);
      next.pc = static_cast<std::uint64_t>(target);
    }
    break;
  }
  }
  if (store)
    memory_[store_address] = store_value;
  state_ = next;
  return state_.halted ? Stop::halted : Stop::running;
}
RunResult Machine::run(std::uint64_t budget) {
  if (state_.fault != Fault::none)
    return {Stop::fault, 0};
  if (state_.halted)
    return {Stop::halted, 0};
  std::uint64_t retired = 0;
  while (retired < budget) {
    const auto result = step();
    if (result == Stop::fault)
      return {result, retired};
    ++retired;
    if (result == Stop::halted)
      return {result, retired};
  }
  return {Stop::step_limit, retired};
}
} // namespace t27::experimental

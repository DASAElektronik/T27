// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "isa.hpp"
#include <array>
#include <span>
#include <vector>

namespace t27::experimental {
enum class Fault {
  none,
  fetch_address,
  illegal_instruction,
  divide_by_zero,
  data_address,
  branch_address,
  stack_overflow,
  stack_underflow
};
enum class Stop { running, halted, fault, step_limit };
struct Flags {
  num::Trit sign{num::Trit::Z};
  bool overflow{false}, inexact{false};
  bool operator==(const Flags &) const = default;
};
struct State {
  std::array<num::Tword27, register_count> registers{};
  std::uint64_t pc{0};
  std::uint64_t sp{0}; // Next occupied stack word; empty at stack_region().end.
  Flags flags{};
  bool halted{false};
  Fault fault{Fault::none};
};
struct StackRegion {
  std::uint64_t begin, end; // Half-open word addresses.
  bool operator==(const StackRegion &) const = default;
};
struct RunResult {
  Stop reason;
  std::uint64_t retired;
};

/// Deterministic, instruction-level model; not a cycle-accurate or physical CPU.
class Machine {
public:
  explicit Machine(std::span<const num::Tword27> image, std::size_t memory_words = 256);
  const State &state() const noexcept {
    return state_;
  }
  std::span<const num::Tword27> memory() const noexcept {
    return memory_;
  }
  StackRegion stack_region() const noexcept {
    return stack_;
  }
  // Host setup: validates bounds, empties the stack, retains memory and other state.
  void configure_stack(std::uint64_t begin, std::uint64_t end);
  void reset(std::uint64_t entry =
                 0); // Clears registers/flags/fault and empties stack; retains memory/bounds.
  void set_register(std::size_t index, const num::Tword27 &value);
  void poke(std::size_t address, const num::Tword27 &value);
  Stop step();
  RunResult run(std::uint64_t instruction_budget);

private:
  State state_{};
  StackRegion stack_{};
  std::vector<num::Tword27> memory_;
};
} // namespace t27::experimental

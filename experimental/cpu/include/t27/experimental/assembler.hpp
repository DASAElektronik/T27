// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "isa.hpp"
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace t27::experimental {
class AssemblyError : public std::runtime_error {
public:
  AssemblyError(std::size_t line, std::size_t column, const std::string &message);
  std::size_t line() const noexcept {
    return line_;
  }
  std::size_t column() const noexcept {
    return column_;
  }

private:
  std::size_t line_, column_;
};
struct Assembly {
  std::vector<num::Tword27> words;
  std::vector<std::size_t> source_lines; // One 1-based source line per emitted word.
};
/// Two-pass ASCII assembler. Throws AssemblyError; never returns a partial image.
Assembly assemble(std::string_view source);
} // namespace t27::experimental

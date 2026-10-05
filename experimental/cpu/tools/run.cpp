// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/assembler.hpp"
#include "t27/experimental/cpu.hpp"
#include "t27/num/convert.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
using namespace t27::experimental;
namespace {
std::uint64_t positive(std::string_view text) {
  std::uint64_t value = 0;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size() || value == 0)
    throw std::invalid_argument("option requires a positive decimal integer");
  return value;
}
const char *fault_name(Fault fault) {
  switch (fault) {
  case Fault::io_endpoint:
    return "io_endpoint";
  case Fault::stack_overflow:
    return "stack_overflow";
  case Fault::stack_underflow:
    return "stack_underflow";
  case Fault::none:
    return "none";
  case Fault::fetch_address:
    return "fetch_address";
  case Fault::illegal_instruction:
    return "illegal_instruction";
  case Fault::divide_by_zero:
    return "divide_by_zero";
  case Fault::data_address:
    return "data_address";
  case Fault::branch_address:
    return "branch_address";
  }
  return "unknown";
}
std::string read_file(const std::string &path, const std::string &kind) {
  std::ifstream input(path, std::ios::binary);
  if (!input)
    throw std::runtime_error("cannot open " + kind + " file: " + path);
  std::string source;
  std::array<char, 4096> buffer{};
  while (input.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) || input.gcount()) {
    source.append(buffer.data(), static_cast<std::size_t>(input.gcount()));
    if (source.size() > 1048576)
      throw std::runtime_error(kind + " exceeds 1 MiB limit");
  }
  if (!input.eof())
    throw std::runtime_error("cannot read " + kind + " file: " + path);
  return source;
}
std::vector<t27::num::Tword27> input_words(const std::string &path) {
  std::istringstream input(read_file(path, "input"));
  input.imbue(std::locale::classic());
  std::vector<t27::num::Tword27> words;
  std::string token;
  while (input >> token) {
    const auto text = std::string_view(token);
    auto digits = text;
    if (!digits.empty() && digits.front() == '+')
      digits.remove_prefix(1);
    if (digits.empty() || (text.front() == '+' && (digits.front() < '0' || digits.front() > '9')))
      throw std::invalid_argument("input requires signed decimal words");
    std::int64_t n = 0;
    const auto [end, error] = std::from_chars(digits.data(), digits.data() + digits.size(), n);
    if (error != std::errc{} || end != digits.data() + digits.size() || n < -word_limit ||
        n > word_limit)
      throw std::invalid_argument("input word outside signed 27-trit decimal range");
    if (words.size() == 65536)
      throw std::invalid_argument("input limit is 65536 words");
    t27::num::Tword27 word{};
    const auto trits = t27::num::to_bt(n);
    std::copy(trits.begin(), trits.end(), word.t.begin());
    words.push_back(word);
  }
  return words;
}
void usage() {
  std::cout
      << "Usage: t27_run FILE [--steps N] [--memory N] [--input FILE] [--output-limit N]\n"
         "ISA v0.2 assembly; default 100000 instructions, at least 256 memory words.\n"
         "Limits: 1 MiB per file, 65536 input words, 1048576 memory/output words.\n"
         "Input is closed after loading (default empty); output capacity defaults to 4096.\n"
         "Output uses OUT[index]=value lines; full output stops with exit 5. Entry is zero.\n";
}
} // namespace
int main(int argc, char **argv) {
  if (argc == 2 && std::string_view(argv[1]) == "--help") {
    usage();
    return 0;
  }
  if (argc < 2) {
    usage();
    return 2;
  }
  const std::string path = argv[1];
  try {
    std::uint64_t budget = 100000, memory = 0, output_limit = 4096;
    std::string input_path;
    bool seen_input = false, seen_output = false;
    bool seen_steps = false, seen_memory = false;
    for (int n = 2; n < argc; n += 2) {
      const std::string_view option = argv[n];
      if (n + 1 == argc)
        throw std::invalid_argument("missing option value");
      if (option == "--steps" && !seen_steps) {
        budget = positive(argv[n + 1]);
        seen_steps = true;
      } else if (option == "--memory" && !seen_memory) {
        memory = positive(argv[n + 1]);
        seen_memory = true;
      } else if (option == "--input" && !seen_input) {
        input_path = argv[n + 1];
        seen_input = true;
      } else if (option == "--output-limit" && !seen_output) {
        output_limit = positive(argv[n + 1]);
        seen_output = true;
      } else
        throw std::invalid_argument("unknown or repeated option: " + std::string(option));
    }
    if (memory > 1048576)
      throw std::invalid_argument("memory limit is 1048576 words");
    if (output_limit > 1048576)
      throw std::invalid_argument("output limit is 1048576 words");
    const auto source = read_file(path, "source");
    const auto incoming = seen_input ? input_words(input_path) : std::vector<t27::num::Tword27>{};
    const auto assembly = assemble(source);
    if (assembly.words.empty())
      throw std::invalid_argument("source emits no words");
    if (!seen_memory)
      memory = std::max<std::uint64_t>(256, assembly.words.size());
    if (memory < assembly.words.size())
      throw std::invalid_argument("memory is smaller than program image");
    Machine machine(assembly.words, static_cast<std::size_t>(memory),
                    {incoming.size(), static_cast<std::size_t>(output_limit)});
    if (!machine.feed_input(incoming))
      throw std::runtime_error("initial input rejected");
    machine.close_input();
    const auto result = machine.run(budget);
    const auto &state = machine.state();
    const char *reason = result.reason == Stop::halted        ? "halted"
                         : result.reason == Stop::fault       ? "fault"
                         : result.reason == Stop::input_wait  ? "input_wait"
                         : result.reason == Stop::output_wait ? "output_wait"
                                                              : "step_limit";
    std::cout << "stop=" << reason << " pc=" << state.pc << " retired=" << result.retired << '\n';
    std::cout << "sp=" << state.sp << " stack_begin=" << machine.stack_region().begin
              << " stack_end=" << machine.stack_region().end << '\n';
    for (std::size_t n = 0; n < register_count; ++n)
      std::cout << "R" << n << "=" << t27::num::from_bt(state.registers[n].span()) << '\n';
    std::cout << "sign=" << static_cast<int>(state.flags.sign)
              << " overflow=" << state.flags.overflow << " inexact=" << state.flags.inexact << '\n';
    std::cout << "input_remaining=" << machine.io().input.size()
              << " input_closed=1 output_count=" << machine.io().output.size() << '\n';
    for (std::size_t n = 0; n < machine.io().output.size(); ++n)
      std::cout << "OUT[" << n << "]=" << t27::num::from_bt(machine.io().output[n].span()) << '\n';
    if (result.reason == Stop::fault) {
      std::cerr << path;
      if (state.pc < assembly.source_lines.size())
        std::cerr << ':' << assembly.source_lines[static_cast<std::size_t>(state.pc)];
      std::cerr << ": fault=" << fault_name(state.fault) << " at pc=" << state.pc << '\n';
      return 3;
    }
    return result.reason == Stop::halted ? 0 : result.reason == Stop::step_limit ? 4 : 5;
  } catch (const AssemblyError &error) {
    std::cerr << path << ':' << error.line() << ':' << error.column() << ": " << error.what()
              << '\n';
  } catch (const std::exception &error) {
    std::cerr << path << ": " << error.what() << '\n';
  }
  return 2;
}

// SPDX-License-Identifier: Apache-2.0
#include "t27/experimental/assembler.hpp"
#include "t27/num/convert.hpp"
#include "t27/num/fixed.hpp"
#include <charconv>
#include <map>
#include <utility>

namespace t27::experimental {
AssemblyError::AssemblyError(std::size_t line, std::size_t column, const std::string &message)
    : std::runtime_error(message), line_(line), column_(column) {}
namespace {
bool letter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
bool digit(char c) {
  return c >= '0' && c <= '9';
}
std::string upper(std::string s) {
  for (auto &c : s)
    if (c >= 'a' && c <= 'z')
      c = static_cast<char>(c - 'a' + 'A');
  return s;
}
struct Token {
  std::string text;
  std::size_t column;
};
struct Line {
  std::vector<Token> tokens;
  std::size_t number, end;
};
using Labels = std::map<std::string, std::int64_t>;
Line lex(std::string_view s, std::size_t number) {
  Line line{{}, number, s.size() + 1};
  for (std::size_t n = 0; n < s.size();) {
    const char c = s[n];
    if (c == '#' || c == ';') {
      line.end = n + 1;
      break;
    }
    if (c == ' ' || c == '\t' || c == '\r') {
      ++n;
      continue;
    }
    const auto start = n++;
    if (letter(c) || c == '.') {
      while (n < s.size() && (letter(s[n]) || digit(s[n])))
        ++n;
    } else if (digit(c)) {
      while (n < s.size() && digit(s[n]))
        ++n;
    } else if (c != ',' && c != ':' && c != '[' && c != ']' && c != '+' && c != '-') {
      throw AssemblyError(number, start + 1, "unexpected character");
    }
    line.tokens.push_back({std::string(s.substr(start, n - start)), start + 1});
  }
  return line;
}
class Parser {
public:
  Parser(const Line &line, const Labels &labels, std::int64_t address)
      : line_(line), labels_(labels), address_(address) {}
  num::Tword27 word() {
    const auto op = upper(take().text);
    if (op == ".WORD") {
      const auto value = integer();
      finish();
      if (value < -word_limit || value > word_limit)
        fail(".word outside signed 27-trit range", 0);
      return num::to_word27(num::to_bt(value));
    }
    Instruction i{};
    if (op == "NOP")
      i.opcode = Opcode::nop;
    else if (op == "HALT")
      i.opcode = Opcode::halt;
    else if (op == "RET")
      i.opcode = Opcode::ret;
    else if (op == "POP") {
      i.opcode = Opcode::pop;
      i.rd = reg();
    } else if (op == "PUSH" || op == "JMPR" || op == "CALLR") {
      i.opcode = op == "PUSH" ? Opcode::push : op == "JMPR" ? Opcode::jmpr : Opcode::callr;
      i.rs1 = reg();
    } else if (op == "LI") {
      i.opcode = Opcode::li;
      i.rd = reg();
      expect(",");
      i.immediate = value(false);
    } else if (op == "MOV") {
      i.opcode = Opcode::mov;
      i.rd = reg();
      expect(",");
      i.rs1 = reg();
    } else if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV" || op == "REM") {
      i.opcode = op == "ADD"   ? Opcode::add
                 : op == "SUB" ? Opcode::sub
                 : op == "MUL" ? Opcode::mul
                 : op == "DIV" ? Opcode::div
                               : Opcode::rem;
      i.rd = reg();
      expect(",");
      i.rs1 = reg();
      expect(",");
      i.rs2 = reg();
    } else if (op == "LOAD") {
      i.opcode = Opcode::load;
      i.rd = reg();
      expect(",");
      memory(i);
    } else if (op == "STORE") {
      i.opcode = Opcode::store;
      memory(i);
      expect(",");
      i.rs2 = reg();
    } else if (op == "JMP" || op == "JZ" || op == "JNZ" || op == "CALL") {
      i.opcode = op == "CALL"  ? Opcode::call
                 : op == "JMP" ? Opcode::jmp
                 : op == "JZ"  ? Opcode::jz
                               : Opcode::jnz;
      if (op == "JZ" || op == "JNZ") {
        i.rs1 = reg();
        expect(",");
      }
      i.immediate = value(true);
    } else
      fail("unknown instruction or directive: " + op, 0);
    finish();
    if (i.immediate < -immediate_limit || i.immediate > immediate_limit)
      fail("immediate outside signed 18-trit range", 0);
    return encode(i);
  }

private:
  const Line &line_;
  const Labels &labels_;
  std::int64_t address_;
  std::size_t pos_{0};
  [[noreturn]] void fail(const std::string &message, std::size_t pos) const {
    throw AssemblyError(line_.number,
                        pos < line_.tokens.size() ? line_.tokens[pos].column : line_.end, message);
  }
  Token take() {
    if (pos_ == line_.tokens.size())
      fail("missing operand", pos_);
    return line_.tokens[pos_++];
  }
  bool accept(std::string_view text) {
    if (pos_ < line_.tokens.size() && line_.tokens[pos_].text == text) {
      ++pos_;
      return true;
    }
    return false;
  }
  void expect(std::string_view text) {
    if (!accept(text))
      fail("expected '" + std::string(text) + "'", pos_);
  }
  void finish() {
    if (pos_ != line_.tokens.size())
      fail("unexpected extra operand or token", pos_);
  }
  std::size_t reg() {
    const auto p = pos_;
    const auto t = upper(take().text);
    if (t.size() != 2 || t[0] != 'R' || t[1] < '0' || t[1] > '8')
      fail("expected register R0 through R8", p);
    return static_cast<std::size_t>(t[1] - '0');
  }
  std::int64_t integer() {
    const auto p = pos_;
    const bool negative = accept("-");
    if (!negative)
      (void)accept("+");
    auto text = take().text;
    if (text.empty() || !digit(text[0]))
      fail("expected decimal integer", p);
    if (negative)
      text.insert(text.begin(), '-');
    std::int64_t n = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), n);
    if (error != std::errc{} || end != text.data() + text.size())
      fail("decimal integer out of range", p);
    return n;
  }
  std::int64_t value(bool relative) {
    if (pos_ < line_.tokens.size() && letter(line_.tokens[pos_].text[0])) {
      const auto p = pos_;
      const auto name = take().text;
      const auto it = labels_.find(name);
      if (it == labels_.end())
        fail("undefined label: " + name, p);
      return relative ? it->second - (address_ + 1) : it->second;
    }
    return integer();
  }
  void memory(Instruction &i) {
    expect("[");
    i.rs1 = reg();
    if (accept("+"))
      i.immediate = value(false);
    else if (accept("-")) {
      // Subtraction accepts an unsigned decimal magnitude, avoiding double signs.
      if (pos_ == line_.tokens.size() || !digit(line_.tokens[pos_].text[0]))
        fail("expected decimal displacement after '-'", pos_);
      i.immediate = -integer();
    }
    expect("]");
  }
};
} // namespace
Assembly assemble(std::string_view source) {
  Labels labels;
  std::vector<Line> lines;
  std::size_t line_number = 1;
  while (!source.empty()) {
    const auto newline = source.find('\n');
    auto line = lex(source.substr(0, newline), line_number++);
    if (newline == std::string_view::npos)
      source = {};
    else
      source.remove_prefix(newline + 1);
    while (line.tokens.size() >= 2 && line.tokens[1].text == ":") {
      const auto &t = line.tokens[0];
      if (!letter(t.text[0]))
        throw AssemblyError(line.number, t.column, "invalid label name");
      if (!labels.emplace(t.text, static_cast<std::int64_t>(lines.size())).second)
        throw AssemblyError(line.number, t.column, "duplicate label: " + t.text);
      line.tokens.erase(line.tokens.begin(), line.tokens.begin() + 2);
    }
    if (!line.tokens.empty()) {
      if (lines.size() >= static_cast<std::uint64_t>(word_limit) + 1)
        throw AssemblyError(line.number, 1, "image exceeds ISA address range");
      lines.push_back(std::move(line));
    }
  }
  Assembly result;
  result.words.reserve(lines.size());
  result.source_lines.reserve(lines.size());
  for (std::size_t n = 0; n < lines.size(); ++n) {
    result.words.push_back(Parser(lines[n], labels, static_cast<std::int64_t>(n)).word());
    result.source_lines.push_back(lines[n].number);
  }
  return result;
}
} // namespace t27::experimental

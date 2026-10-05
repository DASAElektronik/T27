// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <stdexcept>
#include <string>
inline void t27_check(bool ok, const char *expression, const char *file, int line) {
  if (!ok)
    throw std::runtime_error(std::string(file) + ":" + std::to_string(line) + ": " + expression);
}
#define CHECK(...) t27_check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__, __FILE__, __LINE__)
template <class Exception, class F> void check_throws(F &&f) {
  bool caught = false;
  try {
    f();
  } catch (const Exception &) {
    caught = true;
  }
  CHECK(caught);
}

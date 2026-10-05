# SPDX-License-Identifier: MIT
if(NOT T27_ENABLE_COVERAGE AND NOT T27_ENABLE_LLVM_COVERAGE)
  return()
endif()
find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
if(T27_ENABLE_LLVM_COVERAGE)
  find_program(LLVM_COV llvm-cov REQUIRED)
  find_program(LLVM_PROFDATA llvm-profdata REQUIRED)
  add_custom_target(coverage-llvm
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/coverage.py
      llvm ${CMAKE_BINARY_DIR} $<CONFIG> ${CMAKE_CTEST_COMMAND} ${LLVM_COV} ${LLVM_PROFDATA}
      $<TARGET_FILE:t27_tests> $<TARGET_FILE:t27_contract> $<TARGET_FILE:t27_extra> $<TARGET_FILE:t27_oracle_bridge> $<TARGET_FILE:t27_fuzz_smoke> $<TARGET_FILE:t27_example>
    DEPENDS t27_tests t27_contract t27_extra t27_oracle_bridge t27_fuzz_smoke t27_example VERBATIM)
  add_custom_target(coverage DEPENDS coverage-llvm)
else()
  find_program(LCOV lcov REQUIRED)
  find_program(GENHTML genhtml REQUIRED)
  add_custom_target(coverage-lcov
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/coverage.py
      lcov ${CMAKE_BINARY_DIR} $<CONFIG> ${CMAKE_CTEST_COMMAND} ${LCOV} ${GENHTML}
    DEPENDS t27_tests t27_contract t27_extra t27_oracle_bridge t27_fuzz_smoke t27_example VERBATIM)
  add_custom_target(coverage DEPENDS coverage-lcov)
endif()

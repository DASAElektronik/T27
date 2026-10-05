# SPDX-License-Identifier: MIT
if(NOT T27_ENABLE_COVERAGE AND NOT T27_ENABLE_LLVM_COVERAGE)
  return()
endif()
set(T27_COVERAGE_EXTRA_TARGETS "")
set(T27_COVERAGE_EXTRA_BINARIES "")
if(T27_BUILD_EXPERIMENTAL_CPU)
  list(APPEND T27_COVERAGE_EXTRA_TARGETS t27_cpu_tests t27_cpu_demo t27_assembler_tests t27_run)
  list(APPEND T27_COVERAGE_EXTRA_BINARIES $<TARGET_FILE:t27_cpu_tests> $<TARGET_FILE:t27_cpu_demo> $<TARGET_FILE:t27_assembler_tests> $<TARGET_FILE:t27_run>)
endif()
find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
if(T27_ENABLE_LLVM_COVERAGE)
  find_program(LLVM_COV llvm-cov REQUIRED)
  find_program(LLVM_PROFDATA llvm-profdata REQUIRED)
  add_custom_target(coverage-llvm
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/coverage.py
      llvm ${CMAKE_BINARY_DIR} $<CONFIG> ${CMAKE_CTEST_COMMAND} ${LLVM_COV} ${LLVM_PROFDATA}
      $<TARGET_FILE:t27_tests> $<TARGET_FILE:t27_contract> $<TARGET_FILE:t27_extra> $<TARGET_FILE:t27_oracle_bridge> $<TARGET_FILE:t27_fuzz_smoke> $<TARGET_FILE:t27_example> ${T27_COVERAGE_EXTRA_BINARIES}
    DEPENDS t27_tests t27_contract t27_extra t27_oracle_bridge t27_fuzz_smoke t27_example ${T27_COVERAGE_EXTRA_TARGETS} VERBATIM)
  add_custom_target(coverage DEPENDS coverage-llvm)
else()
  find_program(LCOV lcov REQUIRED)
  find_program(GENHTML genhtml REQUIRED)
  add_custom_target(coverage-lcov
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/coverage.py
      lcov ${CMAKE_BINARY_DIR} $<CONFIG> ${CMAKE_CTEST_COMMAND} ${LCOV} ${GENHTML}
    DEPENDS t27_tests t27_contract t27_extra t27_oracle_bridge t27_fuzz_smoke t27_example ${T27_COVERAGE_EXTRA_TARGETS} VERBATIM)
  add_custom_target(coverage DEPENDS coverage-lcov)
endif()

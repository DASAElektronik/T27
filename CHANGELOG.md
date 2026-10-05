<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Changelog

## Experimental ISA v0.1 - unreleased

- Add a checked CPU runtime benchmark, replay fixtures, mutation-sensitivity
  experiments and a documented ISA/ABI and I/O investigation.
- Add an independent Python CPU oracle, deterministic generated programs, full
  architectural snapshots and replayable differential failures.

- Preserve all v0 opcode encodings; add CALL, CALLR, RET, PUSH, POP and JMPR.
- Add a dedicated stack pointer, configurable bounds, precise stack faults and
  validation of return addresses before state changes.
- Define a minimal register calling convention and add recursive factorial and
  indirect-call examples, assembler syntax, runner output and regression tests.

## Experimental ISA v0 - unreleased

- Add a two-pass text assembler with labels, `.word`, source diagnostics and a
  bounded command-line runner; provide four runnable assembly examples.

- Add an opt-in CPU model with nine 27-trit registers, 14 canonical instructions,
  unified word memory, arithmetic flags, precise faults and resumable step budgets.
- Add independent encoding/arithmetic checks and sum, factorial and division demos.
- Exercise the experimental module in CI while excluding it from core installation.
- Document the proposed ISA and the decisions required before freezing it.

## 0.2.0 review candidate - unreleased

Based on the uploaded T27-Project.zip (CMake 0.1.1), SHA256
`aec421d514c18c2ced014df2278f075530c08da05c9b688a2f825e2ed4da745e`.

- Replace nonterminating greedy division and int64 fallback with bounded
  shifted-divisor arithmetic on balanced-trit vectors; replace invalid proof sketch.
- Unify word arithmetic modulo 3^27 and zero-filled right shifts; separate inexact.
- Check int64 range without overflowing; remove __int128 dependency.
- Normalize zero/text formats and reject invalid input; retain explicit LSB utility formats.
- Fix negative rounding tie oracles, header completeness and extra test integration.
- Add independent Python arbitrary-precision tests, contract/header tests and an example.
- Keep assertions active in Release; connect sanitizers, coverage, fuzz and packaging.
- Consolidate build on CMake, fix preset schema, release artifact names and permissions.
- Remove obsolete integration patches and manually duplicated VS projects.
- Correct credits, citations and current-scope descriptions; retain inherited license notices.

See docs/guide/migration-0.2.md for all intentional compatibility changes.

## Repository integration

Preserve src/t27-core and examples/bench; add the real root CMake build.
Unify MkDocs and Doxygen, correct main-branch links, and preserve master-only
floating-point material as RFCs outside installed headers.

# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased]
### Added
-

### Changed
-

### Fixed
-

## [0.1.0] - 2025-09-28
- Initial public scaffolding

<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Changelog

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

See docs/migration-0.2.md for all intentional compatibility changes.

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

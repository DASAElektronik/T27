<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# T27 - Balanced-Ternary Core

[![Core CI](https://github.com/DASAElektronik/T27/actions/workflows/ci.yml/badge.svg)](https://github.com/DASAElektronik/T27/actions/workflows/ci.yml)
[![docs](https://github.com/DASAElektronik/T27/actions/workflows/docs.yml/badge.svg)](https://github.com/DASAElektronik/T27/actions/workflows/docs.yml)

**0.2.0 review candidate, not a published release.** This revision consolidates the
integer core and its independent tests with the public repository and documentation.
See the [project site](https://dasaelektronik.github.io/T27/).

T27 is an open research project led by Daniel Schuch. The current deliverable is a
C++20 reference for balanced-ternary **integers** (`-1, 0, +1`) and 27-trit words.
CPU/ISA, FPGA hardware, fixed-point, stochastic computing and an OS remain future work.
AI-assisted analysis and development using OpenAI ChatGPT/Codex are described in
[credits](docs/guide/credits.md).

## Build and test

Requires CMake 3.20+, a C++20 compiler, and Python 3.9+ for the independent integer
oracle. Python uses only its standard library. Doxygen is optional.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DT27_BUILD_TESTS=ON
cmake --build build --config Debug --parallel 2
ctest --test-dir build -C Debug --output-on-failure
```

### Visual Studio 2022

Use **File > Open > Folder** on this folder, then select `vs2022-debug` from
CMakePresets.json. The old handwritten `.sln`/`.vcxproj` copies have been removed;
CMake now defines all test targets, including header checks and the Python oracle.

From a VS2022 developer PowerShell:

```powershell
cmake --preset vs2022-debug
cmake --build --preset build
ctest --preset ctest
cmake --build --preset release
ctest --preset ctest-release
```

Python must be available during CMake configuration. If missing, CMake warns that
the independent oracle will not run; such a run does not satisfy the release gate.

## Core contract

- Vectors are LSB-first; all normal text interfaces are **MSB-first** using `-0+`.
- Canonical zero is `[Z]`. Empty input spans are accepted as numeric zero; empty
  text and invalid symbols/trit values are rejected.
- Fixed arithmetic wraps symmetrically modulo `3^27` and reports `overflow`.
- Both right-shift names zero-fill and round division by odd `3^k` to the nearest
  integer. Lost nonzero trits set `inexact`; no binary sign extension is used.
- Division truncates toward zero: `a=q*b+r`, `abs(r)<abs(b)`, remainder sign follows
  the dividend. All widths use the same terminating trit-vector algorithm.
- Explicit Euclid, floor, ceil, nearest-away and nearest-even APIs are available.
- `from_bt` detects out-of-range int64 values without overflowing intermediate values.

Read [the normative contract](docs/design/numeric-contract.md) and
[the migration guide](docs/guide/migration-0.2.md) before replacing an older snapshot.

## Validation

CTest registers basic, contract, extra, example, deterministic fuzz-harness, and
independent Python integer-oracle tests.
Checks remain active in Release. Public headers are compiled individually.

```sh
cmake -S . -B build-san -DCMAKE_BUILD_TYPE=Debug -DT27_ENABLE_SANITIZERS=ON
cmake --build build-san --parallel 2
ctest --test-dir build-san --output-on-failure
python tools/check_install.py build Debug
```

Further options: `T27_BUILD_BENCH`, `T27_BUILD_FUZZ` (Clang/libFuzzer),
`T27_ENABLE_COVERAGE` (gcov/lcov), `T27_ENABLE_LLVM_COVERAGE` (Clang/LLVM).
Coverage formats are mutually exclusive. GCC/Clang sanitizer options fail explicitly
on unsupported compilers rather than silently doing nothing.

## Documentation and packaging

```sh
python -m pip install -r docs/requirements.txt
python tools/build_docs.py  # also requires Doxygen on PATH
```

The combined site is `site/index.html`; generated API docs are `site/api/index.html`.
One workflow validates documentation on PRs and deploys only from `main`.
Floating-point drafts from `master` are preserved under `docs/rfcs/` and are not
installed as part of the integer API.

```sh
cmake --install build --config Debug --prefix install
cd build
cpack -C Debug -G ZIP
```

## Repository layout

- `src/t27-core/`, `include/t27/`: integer implementation and public API.
- `tests/`, `fuzz/`, `examples/`: independent checks and examples.
- `docs/`: MkDocs site, Doxygen inputs and historical RFCs.
- `cmake/`, `tools/`, `.github/workflows/`: build, validation and delivery.

## Licenses and provenance

The public project uses Apache-2.0 for code/HDL, CERN-OHL-P-2.0 for hardware,
and CC BY 4.0 for documentation. The consolidated 2025 integer sources carry an
inherited MIT notice, preserved in `LICENSES/MIT-Core.txt` and `NOTICE`.
See [LICENSE-ROUTING.txt](LICENSE-ROUTING.txt) for provenance and distribution notices.
The project [Patent Pledge](PATENT-PLEDGE.md) and community policies remain in place.

See [the wider project review](docs/repro/review-2026-10-05.md) for the main/master
comparison and floating-point draft findings.

The implementation is experimental research. No patent-clearance, energy-efficiency,
physical ternary hardware or completed CPU/OS claim is made by this software snapshot.

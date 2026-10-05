<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Tests and CI

The root CMake build registers six always-active test groups: basic, contract,
extra, example, deterministic fuzz smoke and independent Python integer oracle.
The oracle uses Python integers, 30,624 cases and operands up to 768 bits; the
fuzz smoke has 1,001 deterministic inputs. Twelve public headers compile alone.

With `T27_BUILD_EXPERIMENTAL_CPU=ON`, six additional groups validate ISA, its examples, assembler, CLI, calls/stack and an independent CPU oracle;
all three experimental headers also compile alone. The CLI group needs Python. Core CI,
sanitisers, coverage and CodeQL enable this option. Release packaging keeps the
core default; installation never includes the experimental API.

| Workflow | Required behavior |
| --- | --- |
| Core CI | Linux/Windows Debug and Release; twelve tests, installed consumer and ZIP package |
| Sanitizers | Actual ASan/UBSan instrumentation and CTest |
| Coverage | Instrumented gcov build, tests and generated lcov report |
| Fuzz | Clang libFuzzer for a fixed time budget; invariant failures fail the job |
| Format | clang-format 18.1.8 on compiled C++ sources |
| docs | Strict MkDocs + Doxygen; PR preview artifact, deploy only from main |
| Release | Matching version/tag, tests before packaging, draft release |

The historical basic-hygiene workflow reports missing SPDX headers but is
informational; it is not a license-compliance gate. See source notices and routing.

## Recorded local candidate evidence

GCC 13.3.0 Debug and Release each passed 6/6 groups. Installation, CPack and
Doxygen passed. ASan/UBSan passed with local leak detection disabled, with the
basic group rerun after relinking a corrupted generated executable.
LeakSanitizer was not validated in that environment. These candidate results do
not imply that hosted CI or MSVC succeeded; use the integration PR checks for that.

See the [review](review-2026-10-05.md) and [tooling](tooling.md).

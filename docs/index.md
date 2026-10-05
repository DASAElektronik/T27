<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Ternary Computer (T27)

T27 is an open research project for balanced ternary (−1, 0, +1).
The current implementation is a **C++20 integer reference**, with arbitrary-length
trit vectors and fixed 27-trit words. Version 0.2.0 is an unreleased review candidate.

- [Build and test the core](guide/quickstart.md)
- [Write and run assembly programs](guide/assembler.md)
- [Run the experimental ISA v0 emulator](design/isa-v0.md)
- [Read the numeric contract](design/numeric-contract.md)
- [Migrate from the 2025 implementation](guide/migration-0.2.md)
- [Browse the generated API](api/index.html)
- [Review the floating-point RFCs](rfcs/float-status.md)

The candidate corrects division termination, multiplication wrap, shifts,
conversions and test references. The [review](repro/review-2026-10-05.md) separates
implemented functionality from designs and records validation limits.

An optional ISA v0 model now executes small programs; its design is experimental.
FPGA, fixed-point, stochastic computing and an OS remain future work.
Energy savings, measured hardware performance and third-party patent clearance
have not been established by the software implementation.

See the [roadmap](roadmap.md), [licensing](guide/licensing.md) and
[contribution guide](guide/contributing.md).

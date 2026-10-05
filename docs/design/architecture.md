<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Architecture and implementation status

The implemented system includes an integer library and an optional experimental
instruction-level CPU model. There is no physical processor implementation.

| Layer | Current state |
| --- | --- |
| Trit value and vector | Implemented; digits −1, 0, +1, vectors LSB-first |
| Exact integer arithmetic | Implemented; checked conversions and terminating vector division |
| Fixed 27-trit words | Implemented; symmetric wrap modulo 3^27, explicit flags |
| Floating-point | [Historical RFCs](../rfcs/float-status.md); open semantics and missing implementations |
| ISA / CPU / emulator | [Experimental ISA v0](isa-v0.md): nine registers, 14 instructions, precise faults |
| HDL / FPGA / physical cells | Planned; no implementation or measurement supplied here |
| Stochastic computing | Separate research track requiring a statistical contract |
| OS | Follows ISA, ABI, boot, I/O and interrupts |

27 trits represent about 42.8 bits of information. The C++ `Tword27` currently
uses 27 bytes on the tested host. A two-bit-per-trit FPGA encoding would use
54 bits and needs its own invalid-code and interface rules. None of these facts
proves an energy/area advantage over matched binary hardware.

The [numeric contract](numeric-contract.md) is the deterministic reference.
The [cells page](cells.md) describes physical encoding possibilities, not measured cells.

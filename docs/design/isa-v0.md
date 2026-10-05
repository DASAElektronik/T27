<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Experimental ISA v0 and emulator

This is an executable design proposal, not a frozen ISA or a hardware implementation.
The normative rules on this page apply to the optional `experimental/cpu` model.
Its headers and library are not installed with the 0.2.0 integer core. Encoding,
API and architecture may change before an ISA is accepted.

## Build and run

```sh
cmake -S . -B build-cpu -DCMAKE_BUILD_TYPE=Debug -DT27_BUILD_TESTS=ON -DT27_BUILD_EXPERIMENTAL_CPU=ON
cmake --build build-cpu --config Debug --parallel 2
ctest --test-dir build-cpu -C Debug --output-on-failure
./build-cpu/t27_cpu_demo
```

With Visual Studio, run `build-cpu/Debug/t27_cpu_demo.exe`. The demo verifies
sum(1..10) = 55, factorial(6) = 720, and −7 / 3 = −2 with remainder −1.
The option defaults to OFF. Enabling it adds `t27_cpu`, `t27_cpu_demo` and the `t27_run` assembler/runner, plus tests and
three isolated header checks when testing is enabled.
See the [assembler guide](../guide/assembler.md) to run text programs.

## Machine state and memory

- Nine general-purpose registers R0–R8, each a signed 27-trit word. No register is
  hardwired to zero. Values range from −3,812,798,742,493 to +3,812,798,742,493.
- A nonnegative word-addressed PC, initially zero. Each instruction occupies one
  word. The default memory has 256 words; construction chooses its size.
- Unified, zero-initialized instruction/data memory. The image is copied at
  address zero. Memory size must be 1 through 3,812,798,742,494 words and is also
  limited by host capacity. Host allocation failures are not architectural faults.
- Three arithmetic flags: sign (−1, 0, +1), overflow and inexact; initially zero.
- A halt latch and a fault latch, initially clear.

Addresses must be in the allocated memory. Effective addresses are calculated
exactly, without 27-trit wrap. A STORE may modify an instruction; the next fetch
observes the new word. There are no alignment restrictions beyond word addressing,
memory-mapped devices, privilege modes, interrupts or concurrency in this model.

## One-word encoding

Fields are balanced-ternary numbers, least-significant trit first. Offsets count
from the least-significant trit of the instruction. This is a logical trit encoding;
no binary file format, byte order or physical voltage encoding is specified.

| Field | Trit offset | Width | Meaning |
| --- | ---: | ---: | --- |
| opcode | 0 | 3 | 0 through 13; negative values reserved |
| rd | 3 | 2 | Destination register index minus 4 |
| rs1 | 5 | 2 | First source register index minus 4 |
| rs2 | 7 | 2 | Second source register index minus 4 |
| immediate | 9 | 18 | Signed integer, −193,710,244 through +193,710,244 |

Used register fields encode R0 as −4, R4 as zero and R8 as +4. **Unused fields
must contain all-zero trits.** Their zero encoding does not imply a reference to
R4. The C++ `Instruction` uses register indices 0–8 and requires unused members
to be zero. `encode` rejects invalid arguments; `decode` rejects reserved opcodes,
nonzero unused fields and invalid trit values with `std::invalid_argument`.

For example, `LI R0, 5` has integer word value `2 + (-4)*27 + 5*19683 = 98309`.
The integer value is an independent way to check the logical field layout.

## Instructions

`a` and `b` below mean the old values of rs1 and rs2. Source and destination
registers may alias. All operands are read before any destination is changed.
Except for HALT and taken branches, successful instructions advance PC by one.

| Value | Instruction | Used fields | Result |
| ---: | --- | --- | --- |
| 0 | NOP | none | Advance |
| 1 | HALT | none | Set halt latch; retain PC at HALT |
| 2 | LI | rd, immediate | rd = immediate, zero-extended to 27 trits |
| 3 | MOV | rd, rs1 | rd = a |
| 4 | ADD | rd, rs1, rs2 | rd = a + b, wrapped modulo 3^27 |
| 5 | SUB | rd, rs1, rs2 | rd = a − b, wrapped modulo 3^27 |
| 6 | MUL | rd, rs1, rs2 | rd = a × b, wrapped modulo 3^27 |
| 7 | DIV | rd, rs1, rs2 | rd = quotient of a / b, truncated toward zero |
| 8 | REM | rd, rs1, rs2 | rd = remainder of a / b; sign follows dividend |
| 9 | LOAD | rd, rs1, immediate | rd = memory[a + immediate] |
| 10 | STORE | rs1, rs2, immediate | memory[a + immediate] = b |
| 11 | JMP | immediate | PC = old PC + 1 + immediate |
| 12 | JZ | rs1, immediate | Branch as JMP if a = 0 |
| 13 | JNZ | rs1, immediate | Branch as JMP if a ≠ 0 |

Balanced-ternary immediates gain zero high trits when widened, including negative
values. Branch conditions inspect the source register, not the sign flag. An
untaken branch does not validate its hypothetical target.

Every register-writing instruction sets sign to the sign of its final stored
result. ADD/SUB/MUL set overflow if the exact result is outside the word range,
and clear inexact. DIV clears overflow and sets inexact iff its remainder is
nonzero. The symmetric range makes all nonzero-divisor quotients representable.
REM, LI, MOV and LOAD clear overflow and inexact. NOP, STORE, branches and HALT
preserve all flags. Arithmetic rules follow the [core contract](numeric-contract.md).

## Faults and execution budget

| Fault | Condition |
| --- | --- |
| fetch_address | PC is outside allocated memory at fetch |
| illegal_instruction | Fetched word fails canonical decoding |
| divide_by_zero | DIV or REM has a zero divisor |
| data_address | LOAD or STORE has an out-of-range effective address |
| branch_address | A taken branch has an out-of-range target |

A trapping instruction changes only the fault latch: registers, flags, PC and
memory retain their prior values. No faulting instruction retires. Faults and
HALT are sticky: subsequent `step`/`run` calls do nothing until `reset`.
Sequential execution can advance PC just beyond memory; that instruction retires,
and the subsequent fetch faults. HALT itself retires and leaves PC at its address.

`run(budget)` reports the stop reason and the number of retired instructions in
that call. Exhausting the budget returns `step_limit` without setting a machine
latch; execution can resume. A zero budget executes nothing, while an existing
halt or fault is still reported. This count represents instructions, not cycles.

`reset(entry)` validates the entry address, clears registers/flags/latches and
retains current memory. It does not restore the original image. Host `poke` and
`set_register` validate indices and trits, and do not affect flags or clear latches.
Host misuse throws `std::out_of_range` or `std::invalid_argument`; it is separate
from faults raised by emulated instructions.

## Example loop

The following notation illustrates the program in `experimental/cpu/examples/demo.cpp`.
The C++ example constructs `Instruction` objects and encodes them into memory words.
For assembler input, omit the numeric address prefixes below. A label-based version
is available in `experimental/cpu/programs/sum.t27`, with an additional LOAD to
verify the stored result.

```text
0: LI    R0, 0
1: LI    R1, 1
2: LI    R2, 11
3: LI    R3, 1
4: ADD   R0, R0, R1
5: ADD   R1, R1, R3
6: SUB   R4, R1, R2
7: JNZ   R4, -4          # next PC 8 minus 4 = 4
8: STORE [R8 + 40], R0   # R8 is zero after reset
9: HALT
```

This retires 46 instructions and stores 55 at memory word 40.

## Validation and decisions still open

Tests check independent numeric encoding vectors, all 3,645 arithmetic register
tuples, immediate limits, reserved encodings, 8,323 small signed arithmetic cases,
word overflow boundaries, register aliasing, loops, self-modifying code, budgets,
host validation and atomic faults. The existing core oracle remains independent.

Before freezing an ISA, decide register count, opcode allocation, full-word constant
construction, indirect jumps, stack/call conventions, executable serialization and
versioning. A text assembler and runner are now implemented; ABI, boot contract, I/O and
interrupts remain subsequent milestones. FPGA work should compare each architectural transition against this
model only after the relevant ISA decisions have been accepted.

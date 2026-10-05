<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Write and run experimental T27 programs

The experimental assembler translates text into the same canonical instruction
words used by the [ISA v0 emulator](../design/isa-v0.md). `t27_run` assembles a
file and executes it from address zero. No separate assembler installation is needed.
The [ISA v0.1 extension](../design/isa-v0.1.md) adds CALL/CALLR, RET, PUSH/POP
and JMPR, with recursive and indirect-call examples.
Both tools remain part of the optional CPU module, outside the installed core API.

## Build and try it

From the repository root:

```sh
cmake -S . -B build-cpu -DCMAKE_BUILD_TYPE=Debug -DT27_BUILD_EXPERIMENTAL_CPU=ON -DT27_BUILD_TESTS=ON
cmake --build build-cpu --config Debug --parallel 2
ctest --test-dir build-cpu -C Debug --output-on-failure
./build-cpu/t27_run experimental/cpu/programs/sum.t27
```

On Windows with Visual Studio 2022, use the same CMake commands and then:

```powershell
.\build-cpu\Debug\t27_run.exe experimental/cpu/programs/sum.t27
```

The sum example halts after 47 instructions with `R0=55` and `R5=55`. Other
checked-in examples are `factorial.t27` (R0 = 720), `division.t27` (R2 = −2,
R3 = −1), and `constants.t27` (full-word constant and overflow).

## Source syntax

Use ASCII source, one instruction or directive per line. LF and CRLF line endings
are supported. Blank lines, spaces and tabs are allowed; `;` and `#` start comments
through the end of the line. Mnemonics, `.word` and registers are case-insensitive.
Labels are case-sensitive and use `[A-Za-z_][A-Za-z0-9_]*` followed by `:`.
A label names the address of the next emitted word. It may share a line with an
instruction or stand alone; multiple labels may name the same address.

Operands require commas and memory operands require brackets. Numbers are decimal,
with an optional leading `+` or `-`. Hexadecimal, floating-point, ternary text,
expressions, macros and includes are not supported. Each instruction or `.word`
emits exactly one word, without implicit padding or a trailing HALT.

| Form | Meaning |
| --- | --- |
| `NOP`, `HALT` | No operands |
| `LI R0, -7` | Load a signed 18-trit immediate |
| `LI R0, data` | Load the absolute word address of a label |
| `MOV R0, R1` | Copy a register |
| `ADD R0, R1, R2` | Three-register arithmetic; same form for SUB/MUL/DIV/REM |
| `LOAD R0, [R1]` | Load from address R1 |
| `LOAD R0, [R1 + 4]` | Load from R1 plus displacement |
| `STORE [R1 - 4], R0` | Store with a negative displacement |
| `LOAD R0, [R8 + data]` | Add a label's absolute address to the base register |
| `JMP loop` | Branch to a label |
| `JZ R0, done`, `JNZ R0, loop` | Branch on a register value |
| `JMP -1` | Numeric operands are relative displacements; here, branch to itself |
| `.word -3812798742493` | Emit one signed 27-trit data word |

For label branches, the assembler computes `label_address - (instruction_address + 1)`.
For LI and memory displacements, labels give absolute word addresses. R8 is not a
hardware zero register: it starts at zero and remains usable as a zero base only
while your program leaves it unchanged. `[Rn + signed_number]` is accepted;
`[Rn - magnitude]` requires unsigned decimal digits. Subtracting labels and
label arithmetic such as `label+1` are not supported.

Immediates and displacements must fit −193,710,244 through +193,710,244. `.word`
accepts only a decimal integer in −3,812,798,742,493 through +3,812,798,742,493.
A data label with LOAD lets programs use full-word constants without changing
ISA v0. Execution must explicitly halt or branch around data:

```text
LOAD R0, [R8 + maximum]
LI R1, 1
ADD R2, R0, R1
HALT
maximum: .word 3812798742493
```

This leaves R2 at −3,812,798,742,493 and sets overflow. Data words are not tagged;
executing one interprets it as an instruction and may fault. A label after the
last word is allowed, but naming an address does not make that address executable.
Memory and branch validity are checked by the emulator when the instruction runs.

## Execution and diagnostics

```sh
./build-cpu/t27_run experimental/cpu/programs/factorial.t27 --steps 1000 --memory 512
```

The default execution budget is 100,000 retired instructions. Default memory is
at least 256 words, growing to fit the image. `--memory` chooses an explicit size
that must fit the image. Both options require positive decimal integers and may
appear once, in either order, after the file name. Use `--help` for usage.
The runner accepts up to 1 MiB of source and 1,048,576 memory words. These are
runner limits, not new ISA limits. The C++ assembler API has no 1 MiB cap.

The runner prints stop reason, PC, retired instruction count, all nine registers, stack pointer/bounds
and flags. Assembly errors report `file:line:column: message`, with 1-based source
positions (tabs count as one character). Runtime faults report their name and PC,
plus the original source line when that PC belongs to the assembled image. For
self-modifying code this line refers to the original source, not the stored replacement.

| Exit code | Result |
| ---: | --- |
| 0 | Program reached HALT, or help was displayed |
| 2 | Invalid arguments, input/I/O error or assembly failure |
| 3 | Emulated machine fault |
| 4 | Instruction budget exhausted |

Example error: `bad.t27:1:4: expected register R0 through R8` for `LI R9, 1`.
A budget stop reports the current state, so an infinite loop does not hang the
runner indefinitely under its default budget. The runner does not save a resumable
process image; the C++ Machine API supports resuming execution in memory.

## C++ interface and validation

Include `<t27/experimental/assembler.hpp>` and link the build-tree target `t27_cpu`.
`assemble(string_view)` returns an `Assembly` with `words` and matching 1-based
`source_lines`. It first resolves labels, then validates and encodes operands.
Syntax errors throw `AssemblyError`, carrying `line()`, `column()` and `what()`;
no partial image is returned. Empty or comment-only input returns an empty image;
the command-line runner rejects an empty image.

CTest adds `t27.assembler` for instruction forms, label resolution, numeric
boundaries and diagnostics, and `t27.assembler_cli` for real files, example outcomes,
faults, budgets and exit codes. The CLI test requires Python 3.9+.
The experimental build now has twelve test groups and fifteen isolated header checks
when Python is available. The six core test groups remain unchanged. `t27.calls` tests the call/stack extension.

Indirect calls and a minimal calling convention are implemented in v0.1.
There is no serialized executable/object format, linker, boot service or device I/O
yet. Assembler syntax and the ISA remain experimental.

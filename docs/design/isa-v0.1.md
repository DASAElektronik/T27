<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Experimental ISA v0.1: calls, stack and calling convention

ISA v0.1 extends [ISA v0](isa-v0.md) with six instructions and a bounded stack.
The existing fourteen encodings and their arithmetic behavior are unchanged.
This is an executable proposal, not a frozen hardware ISA or binary ABI. The core
library version remains 0.2.0; CPU ISA versions are a separate experimental track.

## Encoding and instructions

The 27-trit field layout and register bias remain unchanged. Six negative opcode
values that were reserved in v0 are now defined; −13 through −7 remain reserved.
Unused fields still require all-zero trits. The assembler emits one word per
instruction, with no pseudo-instruction expansion.

| Opcode | Assembly | Used fields | Effect |
| ---: | --- | --- | --- |
| −1 | `CALL label` or `CALL displacement` | immediate | Push old PC + 1; branch to old PC + 1 + displacement |
| −2 | `RET` | none | Pop a word and branch to its absolute address |
| −3 | `PUSH R0` | rs1 | Push the old source register value |
| −4 | `POP R0` | rd | Pop a word into the destination register |
| −5 | `JMPR R0` | rs1 | Branch to the old register value as an absolute word address |
| −6 | `CALLR R0` | rs1 | Push old PC + 1; branch to the old register value as an absolute address |

CALL label resolution is `label_address - (instruction_address + 1)`, exactly as
for JMP. Numeric CALL operands are relative displacements. CALLR/JMPR take a
register only; `LI R4, function` or LOAD can prepare an absolute address.
CALL retains the signed 18-trit displacement limit. Register targets may use the
full positive 27-trit range, subject to allocated memory.

All six new instructions **preserve sign, overflow and inexact**, including POP.
This lets a function restore saved registers without erasing its result flags.
CALL/CALLR do not alter general-purpose registers. No register is automatically
saved by a call; PUSH/POP explicitly save values when needed.

## Stack state

SP is a dedicated architectural word-address pointer, separate from R0–R8. The
emulator stores it as `State::sp`. Stack bounds are a half-open interval
`[begin, end)` inside allocated memory, with `begin <= SP <= end` at all times.
The empty stack has SP = end; the full stack has SP = begin. End and an empty SP
may be one past the last memory address. SP is not a signed arithmetic register
and never wraps. Its eventual hardware representation is not specified here.

PUSH/CALL/CALLR decrement SP and write `memory[SP]`. POP/RET read `memory[SP]`
and increment SP. Popping does not erase memory. A return address is an ordinary
27-trit word. There is no hidden return stack, call-depth counter or type tag:
a program must balance saved values before RET.

By default, stack bounds are `[image_word_count, memory_word_count)`. This keeps
stack operations out of the initially loaded code and data. An image filling
all memory has a zero-capacity stack: v0 programs can still run, but a valid call
or PUSH faults with `stack_overflow`. The runner's default memory is still at
least 256 words; use `--memory N` to provide more stack space.

The host API `configure_stack(begin, end)` validates the interval and sets SP to
end. It preserves registers, PC, flags, latches and memory; it discards the logical
stack contents. Invalid intervals throw `std::out_of_range` without changing
state or bounds. Empty regions and explicit overlap with program words are
allowed for host-controlled experiments. `stack_region()` returns the bounds.
`reset(entry)` empties the stack while retaining configured bounds and memory.

This is a bounds check for stack instructions, **not memory isolation**. Ordinary
LOAD/STORE and host `poke` can access stack words; an explicitly overlapping
region can overwrite code. Software must avoid placing heap or other live data
in the configured stack region. There is no instruction to directly read/write
SP or change bounds yet, and no frame-pointer or stack-relative addressing mode.

## Precise faults and precedence

| Instruction | Checks, in order |
| --- | --- |
| CALL/CALLR | Target and return address must be inside memory; then stack must have space |
| JMPR | Target must be inside memory |
| PUSH | Stack must have space |
| POP | Stack must contain a word |
| RET | Stack must contain a word; then its top word must be an address inside memory |

Out-of-range targets, including negative values, raise `branch_address`. A CALL
at the final allocated word also raises `branch_address`, even if its target is
valid, because old PC + 1 is not a valid return address. This check occurs before
the stack-capacity check. Exhausted capacity raises `stack_overflow`; reading an
empty stack raises `stack_underflow`.

As for v0, a trapping instruction retires zero instructions and changes **only**
the fault latch. PC, SP, registers, flags and memory remain unchanged. In particular,
RET validates a saved address before popping it. A valid address need not contain
a valid instruction: decoding is checked at the subsequent fetch. Faults remain
sticky until reset. Host stack reconfiguration does not clear a fault.

Ordinary PUSH/POP fall-through at the final memory word follows the v0 rule:
the operation retires, and the following fetch faults. A successful CALL counts
as one instruction, even though it both writes memory and changes PC. Budgets
may stop between CALL and RET; subsequent Machine::run calls resume that state.

## Calling convention proposal

This convention applies to cooperative assembly programs. Hardware does not
check whether a function obeys it, and no linker or executable ABI is claimed.

| State | Convention |
| --- | --- |
| R0–R2 on entry | Up to three integer arguments, in order |
| R0 on return | One integer return value |
| R1–R4 | Caller-saved scratch; caller saves any values needed after the call |
| R5–R8 | Callee-saved; a function restores any it changes |
| SP | Must return to its value immediately before CALL/CALLR |
| Flags | Caller-saved; unspecified after a call unless a function documents them |

R0 is also caller-saved except for its role as the return value. R8 is not a
permanent zero register; callee preservation does not imply that its incoming
value is zero. The stack uses one-word alignment. There are no implicit argument
slots, shadow space, red zone, variadic rules or stack-passed arguments in this
minimal convention. Entry runs at address zero; the top-level program ends with
HALT, not RET, since no caller return address exists.

A function saves a callee-saved register with PUSH and restores it with POP in
reverse order. For nested calls, caller-saved live values also need saving.
The return address remains below these temporary values until they are popped.

## Runnable examples

Build as in the [assembler guide](../guide/assembler.md), then run:

```sh
./build-cpu/t27_run experimental/cpu/programs/recursive-factorial.t27
./build-cpu/t27_run experimental/cpu/programs/indirect-call.t27
```

On Visual Studio, use `.\build-cpu\Debug\t27_run.exe` instead.
The recursive example accepts a nonnegative n in R0, saves each n across the
nested call and returns n! in R0. The checked-in program uses n = 6, returns 720,
retires 54 instructions and restores SP to 256. At maximum depth it uses thirteen
stack words: seven return addresses and six saved arguments. Its image contains
thirteen words, so `--memory 26` is sufficient and `--memory 25` traps precisely
with `stack_overflow`. Arithmetic beyond the word range follows normal wrapping;
negative input is outside this example function's contract.

The indirect-call example calls an increment function through R4, saves/restores
R5, and uses JMPR to reach HALT. It returns R0 = 42, retains R5 = 99 and restores SP.
The runner prints SP and configured bounds alongside its existing machine state.
Exit code 3 covers both new stack faults, with source line and PC diagnostics.

## Acceptance checks and remaining work

`t27.calls` verifies independent numeric encodings, unused-field rejection,
forward/backward CALL labels, LIFO order, exact capacity and empty stacks, full-word
values, indirect targets, return-address corruption, fault precedence, reset,
flag preservation, pause/resume and recursive factorial for n = 0 through 10.
Recursion is checked with exactly enough stack and one word too few; saved registers
and the original program image are verified. CLI tests execute both new examples
and check both stack-fault exit paths. Existing v0 programs remain regression tests.

An [independent Python oracle](../repro/cpu-oracle.md) additionally compares
complete CPU states and memory for directed and generated programs.
The expanded build has twelve CTest groups and fifteen isolated header checks
when Python and the experimental CPU are enabled. Linux/Windows Debug/Release,
sanitizers, coverage and CodeQL continue to include the experimental module.

Next steps: review this convention, then define a minimal I/O contract. Direct
SP access, local stack frames, stack arguments, protection, interrupt frames and
executable format versioning need separate decisions before a larger runtime or OS.

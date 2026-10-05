<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Independent CPU differential tests

The experimental ISA v0.1 emulator is checked against a separate Python integer
model in `experimental/cpu/tests/cpu_reference.py`. That model imports no C++
code, assembler, arithmetic or conversion routines. It decodes balanced fields
with integer modular arithmetic, uses arbitrary-precision products and performs
precise faults by rolling back state. The production CPU instead stages updates.
The models share the documented ISA contract; they are not independently authored
implementations and agreement is not a proof that the contract itself is correct.

`cpu_trace_bridge.cpp` accepts numeric words, initial registers, memory size,
entry, stack bounds and a sequence of execution budgets. It invokes the real
Machine API and emits JSON snapshots. Its numeric/trit conversion is local and
does not use the core conversion functions or assembler. It is test-only, has
bounded input sizes and is not an executable file format or installed API.

The harness compares every snapshot field: PC, SP, all registers, arithmetic
flags, halt/fault latches, stop reason, per-call retirement count and every memory
word. Most runs use single-instruction budgets; mixed budgets additionally test
resumption and batched retirement. Zero budgets and repeated runs after a sticky
HALT/fault are included. Original memory images are generated numerically rather
than through the C++ assembler.

## Reproduce and expand

Build with `T27_BUILD_TESTS=ON` and `T27_BUILD_EXPERIMENTAL_CPU=ON`, then:

```sh
ctest --test-dir build-cpu -C Debug -R t27.cpu_oracle --output-on-failure
python experimental/cpu/tests/cpu_oracle.py build-cpu/t27_cpu_trace_bridge --report build-cpu/oracle.json
python experimental/cpu/tests/cpu_oracle.py build-cpu/t27_cpu_trace_bridge --seed 270102 --random-cases 10000 --report build-cpu/oracle-expanded.json
```

Visual Studio places the bridge at `build-cpu/Debug/t27_cpu_trace_bridge.exe`.
Python 3.9+ is required. The default seed is 270101 and the default adds 1,000
random cases to a fixed directed corpus. The CTest timeout is 180 seconds and
the bridge subprocess timeout defaults to 120 seconds. A corpus SHA-256 records
the exact numeric bridge input for reproducibility; preserve it with the seed,
source commit and Python version.

Directed cases cover all twenty instructions, all seven architectural faults,
both outcomes of JZ/JNZ, arithmetic sign/overflow boundaries and register aliasing,
nonzero unused instruction fields, self-modifying code, corrupt returns and
recursion with sufficient/insufficient stack space. Random cases mix arbitrary
raw words, long arithmetic sequences and control/memory/stack instructions.
The harness fails if any opcode, fault category or conditional branch outcome
has no executed witness. These event counters count attempted instructions and
observed faults; they are not source branch coverage or proof of exhaustive inputs.

Hand-calculated encoding and arithmetic vectors validate the reference model
before the differential pass. Existing assembler and raw invalid-trit tests remain
separate: this numeric protocol represents only valid trit digits and does not
exercise malformed host API arguments, host reset/reconfiguration sequences,
allocation failures or concurrent access.

## Failure replay and architectural traces

On a mismatch, the harness saves the complete case, failing snapshot index,
expected state and actual state to `cpu-oracle-failure.json` in the working
directory (normally the CTest build directory). Replay it with:

```sh
python experimental/cpu/tests/cpu_oracle.py build-cpu/t27_cpu_trace_bridge --replay build-cpu/cpu-oracle-failure.json
```

`--failure PATH` chooses the failure file. Input/process failures are reported
separately and do not fabricate a state mismatch. `--trace PATH` writes verified
JSON Lines with one `input` record containing the complete case (including stack
bounds) before that case’s snapshots. Snapshots contain case name, step index and
budget, useful for a future
RTL comparison. Only snapshots with budget 1 represent single-instruction
transitions; budget 0 changes no state, and larger budgets aggregate execution.
The initial snapshot has a null budget. Integer words are signed logical ternary
values; this trace defines neither physical trit encoding nor cycle timing.

The optional CPU now adds six groups to the six core CTest groups (twelve total),
with fifteen isolated header checks. CI, sanitizers and coverage include this
oracle. The installed integer library remains unchanged.

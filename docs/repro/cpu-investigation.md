<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# CPU investigation: differential testing and runtime baseline

This bounded investigation extends the ISA v0.1 call/stack review. No production
CPU disagreement was found in the tested inputs. It does not freeze the ISA or
establish hardware correctness. The CPU source tested is checkpoint
`c93e218a270b83af3b69ff84e4c8dbc000e8b42e`; this report's commit adds the benchmark,
trace input records and reproducible experiment artifacts without changing CPU
semantics. The complete environment is recorded in `results/environment.json`.

## Differential results

| Seed | Directed cases | Random cases | State comparisons | Retired instructions |
| ---: | ---: | ---: | ---: | ---: |
| 270101 (CI default) | 1,629 | 1,000 | 48,931 | 11,093 |
| 270102 | 1,629 | 10,000 | 360,931 | 76,604 |
| 270103 | 1,629 | 10,000 | 360,931 | 76,286 |

All compared fields matched. Each corpus reached all 20 opcodes, all seven fault
categories and both outcomes of both conditional branches. Across the three
runs there were 770,793 snapshot comparisons and 21,000 generated random cases;
the directed corpus is repeated, so these are not 25,887 unique programs.
Repeated halted/faulted snapshots are included in the comparison count.
Raw reports and exact input hashes are in `results/cpu-oracle-27010{1,2,3}.json`.
Reproduction commands and limits are in the [oracle guide](cpu-oracle.md).

Three deliberate production-source mutations were compiled separately. The
unmodified Python oracle rejected all three, before any random cases were used:

| Mutation | First differing state field |
| --- | --- |
| POP updates arithmetic flags | flags |
| CALL saves old PC instead of PC + 1 | memory |
| A trap advances PC | pc |

The exact replacements and first witnesses are in `results/cpu-mutations.json`.
On Linux with GCC, reproduce after a Release build using:

```sh
python experimental/cpu/tests/check_mutations.py build/release
```

The script writes source copies, binaries, mismatch replays and a report under
`build/release/mutations`. It does not change production sources. These three
selected mutations demonstrate sensitivity to those errors, not a general
mutation score or proof that every implementation defect will be detected.

## Runtime baseline

Enable both `T27_BUILD_EXPERIMENTAL_CPU=ON` and `T27_BUILD_BENCH=ON`, build Release,
and run `build-cpu/t27_cpu_bench` (Visual Studio: `build-cpu/Release/t27_cpu_bench.exe`).
The optional benchmark assembles and allocates once per workload, warms up 20
times, then reports five samples of 2,000 reset/run/result-validation iterations.
It checks the result, retirement count, HALT and restored SP on every iteration.
Memory is retained by reset; all stack values read in the recursive workload
have been written during that iteration. No assembly, allocation or terminal
output occurs inside the timed loop. Reset and result validation ARE included.

Observed on the shared Linux x86-64 host, GCC 13.3 Release without sanitizers:

| Workload | Instructions/run | Median µs/run | Min–max µs/run |
| --- | ---: | ---: | ---: |
| Sum 1…100 | 304 | 127.06 | 119.08–146.39 |
| Recursive 6! | 54 | 16.12 | 15.01–25.49 |
| 100 DIV/REM pairs | 406 | 229.72 | 219.49–290.42 |

`results/cpu-bench.csv` contains the raw nanosecond samples. Host contention,
frequency and cache effects are uncontrolled; these are a reproducible workload
baseline, not statistically established performance differences. There is no
previous CPU benchmark to establish a speedup. Nothing here predicts FPGA clock
rate, instruction cycles, energy or physical ternary advantages. Profile before
choosing optimizations; preserve differential equivalence for any future change.

## ISA and ABI review

| Finding | Consequence / next decision |
| --- | --- |
| Registers and return addresses are signed words; memory addresses are nonnegative | Preserve checked address conversion rather than silently wrapping negative targets. |
| SP may equal memory size, including word-limit + 1 | The hardware SP representation needs a one-past-end value; a signed general register is insufficient at maximum memory size. |
| Calls validate return PC even if the callee never returns | A call in the final word faults by design; tail transfers should use JMPR/JMP. |
| Stack bounds only constrain stack instructions | LOAD/STORE can corrupt returns; no memory protection or privilege claim. |
| SP cannot be read or adjusted by guest code | Current ABI supports cooperative small functions; general local frames, stack arguments and context switching need a separate design. |
| CALL/RET/PUSH/POP preserve flags, but ABI treats flags as caller-saved | Instruction behavior and function-level promises are distinct; callees may execute arithmetic. |
| R8 is callee-saved, not hardwired zero | Addressing through R8 needs an established value, especially in reusable functions. |
| reset retains memory, stack configuration and loaded code | A fresh-process/replay fixture must specify initial memory; reset is not image reload. |

These are constraints of the documented design, not newly found contradictions.
The current convention is usable for the examples but should remain experimental
until local frames, executable versioning and host/guest I/O are specified.
A [minimal I/O proposal](../design/io-proposal.md) is the next design artifact.

## FPGA comparison preparation

`experimental/cpu/tests/recursive-trace-input.json` is a complete replay fixture:
6!, 26 memory words, stack [13,26), 54 single-instruction runs. Generate the checked
trace with:

```sh
python experimental/cpu/tests/cpu_oracle.py build-cpu/t27_cpu_trace_bridge --replay experimental/cpu/tests/recursive-trace-input.json --trace build-cpu/recursive-trace.jsonl
```

`results/recursive-trace.jsonl` records the input, initial state and 54 verified
transitions. It ends at HALT with R0 = 720 and SP = 26. It specifies logical signed
word values and architectural completion boundaries. An RTL adapter must map its
own trit representation and compare at instruction retirement or precise fault,
not every clock cycle. This is a first fixture, not a hardware testbench. The
larger generator can export all opcode/fault cases using the same trace format.

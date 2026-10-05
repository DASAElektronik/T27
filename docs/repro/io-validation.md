<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# ISA v0.2 I/O validation

CPU/ISA baseline: `bb5dae3d6d78adf1312fa8b538325ffb3ce55bdd`. This report's
revision adds the finite-input CLI adapter, examples and additional hand-calculated
oracle checks. Production CPU/queue semantics are unchanged from that checkpoint.
Local environment: Linux x86-64, GCC 13.3.0 Release, warnings as errors, Python
3.12.14. The comparison uses test-only numeric protocol version 2.

## Recorded comparisons

| Seed | Directed cases | Random instruction cases | Random host schedules | Snapshots | Retired |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 270101 | 1,651 | 1,000 | 1,000 | 110,553 | 23,938 |
| 270202 | 1,651 | 5,000 | 5,000 | 493,229 | 104,313 |

All **603,782** snapshots matched, including all CPU and memory fields, both
queues, input closure/capacities and host return values. Each corpus reached all
22 opcodes, eight architectural faults, both conditional branch outcomes, input
and output waits, successful input/output, EOF, and accepted/rejected input batches.
The 1,651 directed cases repeat between runs; stopped states also repeat. These
counts are not a count of unique state transitions or exhaustive coverage.
The two models share the written contract and author; agreement is not proof.

Raw event counters and exact bridge-input hashes are in
`results/io-oracle-270101.json` and `results/io-oracle-270202.json`.
Reproduce after building with the experimental CPU and tests enabled:

```sh
python experimental/cpu/tests/cpu_oracle.py build-cpu/t27_cpu_trace_bridge --report build-cpu/io-default.json
python experimental/cpu/tests/cpu_oracle.py build-cpu/t27_cpu_trace_bridge --seed 270202 --random-cases 5000 --report build-cpu/io-expanded.json
```

The generated family limit applies per family, so `--random-cases 5000` adds
10,000 random cases in total. On Visual Studio use the executable in the selected
configuration directory. Reports preserve the exact corpus SHA-256.

## Sensitivity and replay

All six separately compiled mutations were rejected by the unchanged oracle.
Three new I/O mutations deliberately kept consumed input, overwrote data on EOF,
or advanced PC during an input wait; first disagreements were respectively queue
state, registers and PC. The existing POP-flags, CALL-return-address and trap-PC
mutations were also detected. Exact edits and first witnesses are recorded in
`results/io-mutations.json`; this is selected sensitivity evidence, not a general
mutation score. On GCC/Linux:

```sh
python experimental/cpu/tests/check_mutations.py build/release
```

A complete host-driven fixture is checked in as
`experimental/cpu/tests/io-trace-input.json`. It exercises input wait, zero and
both word limits, output backpressure, draining/resumption, rejected feeding,
EOF and distinct CPU/I/O resets. Replay and export it with:

```sh
python experimental/cpu/tests/cpu_oracle.py build-cpu/t27_cpu_trace_bridge --replay experimental/cpu/tests/io-trace-input.json --trace build-cpu/io-trace.jsonl
```

`results/io-trace.jsonl` contains the versioned input and 26 verified snapshots.
Host actions and aggregate run budgets are explicit. The fixture is suitable as
an architectural reference for a future RTL adapter; it specifies no clock timing.

## CLI and API acceptance

The build has **13 CTest groups** and 15 isolated header compilation checks.
`t27.io` exercises canonical encoding, distinct destinations, flags, full-word
values, all-or-nothing feed, aliased host spans, zero capacities, endpoint fault
precedence, precise waits/faults and independent CPU/I/O lifecycle.
`t27.assembler_cli` runs the actual process: echo and sum, signed limits and zero,
empty/closed input, output backpressure and exit 5, partial output on fault/budget,
malformed decimals, nonexistent input, duplicate options and input/resource limits.
No test needs an interactive terminal or external device.

CodeQL now runs for all pull-request target branches so stacked experimental PRs
receive the same static analysis as PRs directly targeting main. Other CI jobs
continue to cover Linux/Windows Debug/Release, ASan/UBSan, coverage, core fuzzing,
format, documentation, installed consumption and packaging. Exact hosted outcomes
belong to each commit's checks, rather than a claim frozen in this document.

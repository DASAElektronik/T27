<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Experimental ISA v0.2: bounded word-stream I/O

This extension implements the first integer-stream design after the v0.1
calls/stack milestone. All twenty previous instruction encodings remain valid.
It is an experimental instruction-level contract, not a frozen hardware ISA.

## Encoding and guest behavior

| Opcode | Syntax | Used fields | Meaning |
| ---: | --- | --- | --- |
| −7 | `IN Rdata, Rstatus, endpoint` | rd, rs1, immediate | Read one input word, or report EOF |
| −8 | `OUT Rsource, endpoint` | rs1, immediate | Append one output word |

The endpoint is a signed 18-trit decimal literal. Only endpoint 0 exists: a single
input stream and a single output stream. Other endpoint values are canonically
encodable, but execution raises `io_endpoint` before inspecting queue readiness.
Opcodes −13…−9 remain reserved. All unused fields require zero trits.
IN's two destination registers must differ; an alias is `illegal_instruction`
at decode, and is rejected by the encoder/assembler. Rstatus uses the rs1 field
as a destination. OUT reads its source's old value.

Both instructions preserve arithmetic flags, SP and memory. Their exact effects:

| Condition | Effect | Stop / retired |
| --- | --- | --- |
| IN has a buffered word, open or closed | Consume first word, set Rdata to it, Rstatus = 1, PC += 1 | running / 1 |
| IN is empty and closed | Preserve Rdata, set Rstatus = 0, PC += 1 | running / 1 |
| IN is empty and open | No CPU or I/O change | input_wait / 0 |
| OUT has room | Append source word, PC += 1 | running / 1 |
| OUT is full | No CPU or I/O change | output_wait / 0 |
| Unsupported endpoint | Set only fault latch | fault / 0 |

EOF is persistent: another IN on empty closed input retires again and reports
EOF again. Zero and both signed word limits are ordinary data. A successful I/O
instruction at the last memory address retires normally; its next fetch faults.

Waits are resumable stop reasons, not latches or faults. `run(budget)` returns
immediately at a wait with the count of earlier instructions retired in that call.
The waiting instruction consumes no budget. Retrying without changing readiness
returns the same wait. Host code must impose retry/time limits; the CPU never
busy-waits internally. With a zero budget a runnable/waiting machine reports
`step_limit` without inspecting I/O. Existing sticky HALT/fault precedence remains.

## Host interface and ownership

`Machine(image, memory_words, IoConfig{input_capacity, output_capacity})` owns its
queues. Each capacity defaults to 256 words and is fixed for the instance.
Capacities from 0 through 1,048,576 are accepted; larger values throw before
allocation. A zero-capacity output always waits, and zero-capacity input accepts
no nonempty batch but can still be closed to signal EOF. These bounds describe
the emulator resource API, not the physical hardware memory map.

- `feed_input(span)` validates every trit first, then appends the whole batch or
  returns false if closed or insufficient capacity. Rejection never appends a
  prefix. Even an empty batch returns false when closed. Overlapping spans into
  the machine's queues are supported by copying before insertion.
- `close_input()` is idempotent. Existing buffered input is consumed before EOF.
- `drain_output()` returns all queued words in order and clears that queue. A
  copy/allocation failure occurs before clearing, preserving pending output.
- `io()` exposes a read-only queue/state view; `io_config()` returns capacities.
- CPU `reset(entry)` retains both queues, input closure and capacities. It clears
  CPU registers, flags, latches and stack as before. This can intentionally replay
  a program against remaining input; it does not undo prior output.
- `reset_io()` clears both queues and reopens input, preserving capacities and
  all CPU state, including a fault or HALT. No implicit queue reset occurs.

Host setup errors and allocation failures are C++ exceptions, not guest traps.
The API is single-threaded; callers synchronize host actions with execution.
No callbacks, wall-clock reads, operating-system handles or terminal encoding are
part of the ISA. Text/byte adapters belong outside the word-stream contract.

## Validation

`t27.io` checks independent numeric encodings, register combinations, EOF/data
separation, whole-batch rejection, zero capacities, buffered data after closure,
retry without duplicate output, precise waits/faults, reset ownership and flags.
The independent Python integer model also implements queues, host actions and
wait rollback; the differential harness compares every queue word, closure flag,
capacity, host return value and CPU state after each scheduled operation.

Trace protocol version 2 adds initial I/O configuration and tagged host actions:
run, feed, close, drain, CPU reset and I/O reset. JSONL includes the complete case
before its snapshots, with action and optional instruction budget. A host action
snapshot has reason `running` and retired 0 even when the observed CPU has a
sticky latch; inspect the latch or issue run to obtain its stop reason. Historical
v0.1 result files remain evidence for their recorded source revision and protocol;
regenerate new traces with this revision rather than comparing formats blindly.

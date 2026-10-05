<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Minimal deterministic I/O proposal

Historical proposal, now resolved by [ISA v0.2](isa-v0.2.md).
The text below preserves the original design questions and is not the current
execution contract.

Original status: design proposal only. ISA v0.1 still has no I/O instructions, device
addresses, interrupts or host system calls. Reserved opcodes remain reserved.
The first use case is a replayable integer stream, followed by an explicitly
specified text adapter. The existing arithmetic/stack contract remains the base.

## Recommended first experiment

Use explicit input/output operations against host-supplied word queues. Avoid
assigning fixed memory addresses before a memory map exists. Separate data from
status so every signed 27-trit value, including zero, remains valid input.

| Event | Proposed observable behavior |
| --- | --- |
| Input available | Consume exactly one word, write destination, advance PC and retire once. Preserve arithmetic flags. |
| Input empty, stream open | Report a resumable waiting stop. Do not retire or change PC, registers, flags, SP or memory. |
| Input exhausted, stream closed | Return an explicit end-of-input status, never a zero sentinel. Exact status operand/encoding remains open. |
| Output has capacity | Append exactly one source word, advance PC and retire once. Preserve flags. |
| Output full | Report resumable waiting with no architectural or queue change. |
| Invalid endpoint | Raise a precise I/O fault before consuming or emitting anything. |

Open-empty and closed-empty input must remain distinct. A bounded output queue
prevents an infinite guest loop from allocating unlimited host memory. Waiting
stops must immediately return control to the host; they must not busy-wait inside
`run(budget)`. Only successful operations count toward instruction retirement;
existing instruction budgets still bound runnable guest work. The host is
responsible for bounded retries, cancellation and eventual timeout while waiting.

## Decisions required before implementation

1. **Encoding and status:** decide whether status is a second explicit register,
   a separate query operation or another architectural result. Do not overload
   the existing arithmetic flags or data values. Specify aliasing if two register
   operands can be destinations. Assign opcodes only with these semantics fixed.
2. **Endpoints:** begin with one input and one output stream; decide whether the
   encoding reserves an endpoint field for future devices. Unsupported endpoints
   must fault deterministically.
3. **Reset and ownership:** decide whether CPU reset leaves host queues untouched
   and provide a separate explicit I/O reset. Define ownership and capacity at
   construction; avoid arbitrary host callbacks in the first reference model.
4. **Text adapter:** choose an external byte encoding, newline behavior and the
   accepted numeric range. Raw ternary words are not implicitly Unicode or bytes.
5. **Replay and atomicity:** include initial queues, closed/open state, capacities
   and each successful consumption/emission in trace records. A trapped/waiting
   instruction must not partially change either CPU state or a queue.

## Acceptance examples

Implement first in both the CPU and independent integer model, with identical
external input schedules. Required directed programs: integer echo including
zero and both word limits; sum-until-EOF; empty-open input that later receives a
word; closed input with buffered words before EOF; output-full then drained;
invalid endpoint; zero budget; pause/resume without duplicate consumption/output;
and reset with explicitly chosen queue lifecycle. Add a bounded CLI adapter only
after these state transitions are stable.

Memory-mapped devices, terminal interactivity, filesystems, networking, interrupts
and DMA are subsequent designs. The current proposal makes no claim about their
hardware timing or access permissions.

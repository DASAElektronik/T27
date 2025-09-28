# Scope & Non-Infringement Note (Design-Around)

To avoid overlap with third-party patents (e.g., Huawei CN119652311A / WO 2025/060996 A1),
this project intentionally **does not** implement a +1/–1 base cell with the specific
preprocessing and multi-threshold transistor topology described therein.

Instead, we target one or more of the following non-overlapping approaches:
- Ternary logic via resistor ladder + window comparator
- Current-mode / CML-based multi-level signaling
- Time-domain (pulse-width / delay) ternary encoding
- Purely digital dual-line encoding with on-chip encoder/decoder

Maintainers keep this note updated as the architecture evolves.

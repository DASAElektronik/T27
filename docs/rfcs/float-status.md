<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Floating-point RFC status

These drafts are preserved from `master` commit
`8e1f11c5f2e4f6ced3e8fbd511070823f3dd26ad`. They are not part of the supported
integer API or installed headers. The [original draft header](t27float.hpp.txt)
is retained byte-for-byte as a downloadable reference.

The [Ternary27](fp-ternary27.md) and [Ternary6](fp-ternary6.md) documents describe
containers and possible encodings. Pack/unpack, double conversions and total
ordering have declarations, but no implementations in the reviewed branches.

Before implementation, resolve:

1. Exact rational value formula, radix point, normalization and exponent scaling.
2. Negative subnormal values: sign=0 is currently also a subnormal marker.
3. Zero, infinity, NaN payloads and invalid/reserved states. Value-initializing
   the draft container currently does not specify a valid numeric zero.
4. Explicit enum-to-trit mapping, trit order and byte order for a stable encoding.
5. Numeric unordered comparisons versus total ordering; NaN must not accidentally
   compare equal merely because the function returns zero.
6. Validation of raw trit fields and distinction between zero and subnormal.
7. Exhaustive classification of all 729 Ternary6 encodings and external fixed
   encoding vectors, before implementing the larger format.

The integer-core consolidation deliberately makes none of these open choices.
See the [project review](../repro/review-2026-10-05.md) for the wider assessment.

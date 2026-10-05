<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# T27 integer contract - candidate 0.2

This document is normative for this review candidate. Changes to it require tests
and a migration note. It specifies integer arithmetic, not fixed-point or an ISA.

## Representation

A valid `Trit` is `N=-1`, `Z=0`, or `P=+1`. A vector represents
`sum(d[i] * 3^i)` with index zero least significant. Empty spans and leading zero
trits may be passed to numeric APIs. Results use one zero trit for zero and otherwise
no leading zero trits. `Tword27` always stores exactly 27 trits; storage is an array,
not the future FPGA 2-bit wire format. Public arithmetic/conversion entry points
reject invalid enum values. Raw spans/arrays can be modified by callers and are
validated when consumed.

Text is MSB-first, symbols `-0+`, case irrelevant because letters are invalid.
`parse_bt` rejects empty strings and every foreign character, including whitespace.
Leading zeros are accepted and normalized. `util::parse_bt` is the same declaration
as `num::parse_bt`. Explicit `parse_bt_lsb`/`to_string_bt_lsb` provide the old utility
text order; ordinary utility printers use MSB-first.

## Variable arithmetic

`add`, `sub`, `neg`, `mul` compute exact integer results using vectors. Length is
limited by available memory and standard container sizes. No conversion through
int64 is used to implement these operations or division. `cmp` returns N/Z/P.

`to_bt` supports every int64 value, including INT64_MIN. `from_bt` returns an int64
only when representable and otherwise throws `overflow_error`. It performs checked
unsigned-magnitude Horner accumulation; no `__int128` dependency is required.

## Fixed 27-trit arithmetic

Let `M=3^27=7625597484987`, `H=(M-1)/2=3812798742493`.
A word represents the interval `[-H,H]`. For an exact result x, wrapping is
`((x+H) mod M)-H`, with nonnegative mathematical modulo.

`add27`, `sub27`, `mul27`, and `shl27` use this same rule. `overflow` is true exactly
when the exact result is outside `[-H,H]`. `neg27` cannot overflow for a valid word.
`to_word27` is an explicit wrapping conversion with no returned flags;
`pack_word27` also returns the overflow flag. `to_vec` returns a canonical vector.
No cyclic modulo-(3^27-1) multiply is part of this API.

## Shifts and powers of three

Negative k is rejected with `invalid_argument`. Left shift multiplies by `3^k`.
Right shifts `shr` and `lshr` both remove the k least-significant trits and zero-fill.
This equals nearest integer division by `3^k`; since the divisor is odd, integer
inputs never land exactly halfway. It is not truncation-toward-zero division.

`divmod3k_balanced` returns this quotient and the dropped signed remainder, so
`x=q*3^k+r` and `abs(r)<=(3^k-1)/2`. k=0 yields the input and zero remainder.

Word right shifts have `overflow=false`; `inexact` indicates nonzero discarded
trits. For k>=27 the quotient is zero and inexact is true iff the input is nonzero.
Large k is handled without constructing `3^k` in the fixed-word functions.

## Division and rounding

For b!=0, basic division returns the unique truncation pair:
`a=q*b+r`, `abs(r)<abs(b)`, and r=0 or sign(r)=sign(a).
`divmod`, `divmod_long` and `divmod_long_strict` share this contract. The historical
strict CMake options are accepted but no longer change algorithm behavior.

All explicit modes preserve reconstruction `a=q*b+r`:

| Mode | Quotient / remainder rule |
| --- | --- |
| trunc | q toward zero |
| floor | q toward negative infinity |
| ceil | q toward positive infinity |
| euclid | 0<=r<abs(b) |
| nearest-away | nearest q; exact half ties away from zero |
| nearest-even | nearest q; exact half ties to an even integer |

Parity in balanced ternary is the parity of the number of nonzero trits because
every power of 3 and both +/-1 are odd. Tests use an independent integer oracle.

Variable divide-by-zero throws `invalid_argument`. Word wrappers instead return
zero q/r with `divide_by_zero=true`, `overflow_q=false`, `inexact=false`.
Other invalid inputs still throw. For nonzero divisors, word wrappers set inexact
iff the returned remainder is nonzero, and overflow_q iff the exact q is outside
the word interval. For two valid 27-trit operands these quotient rules fit the word
interval; the flag remains explicit for API consistency.

## Status

This is an intentional compatibility break from the archived 0.1.1 snapshot.
It is a reviewed software model running on ordinary binary processors. Mapping
trit operations to an ISA or physical gates is a subsequent design task.

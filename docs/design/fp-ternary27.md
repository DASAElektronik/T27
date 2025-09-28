# Floating Point: Ternary27 (draft)

This page records a pragmatic floating-point format for balanced ternary used in T27.

## Field layout (27 trits total)

```
[ T1 T0 ] [ S ] [ E4 E3 E2 E1 E0 ] [ F18 ... F1 F0 ]
 2 trits   1     5 trits                 19 trits
  type     sign  exponent                significand
```

Conventions:
- Balanced trit alphabet: -, 0, +
- Sign trit S in {-, 0, +}. Real values use S in {-, +}. S == 0 is used for subnormal reals (see below).
- Exponent is balanced with bias 0 (range chosen by spec below).
- Significand is an unsigned magnitude in balanced digits {-1,0,+1} with an implicit leading non-zero after normalization.

### Types (T1 T0)
- `++` : Infinity
- `+0` : Quiet NaN
- `+-` : Signaling NaN
- `0+` : Real, rounded up
- `00` : Real, exact (no rounding happened)
- `0-` : Real, rounded down
- others reserved

### Exponent range
- eMax = +121, eMin = -121 (subject to change while spec is draft).

### Subnormal reals
- A real with S == 0 is interpreted as subnormal; exponent is treated as eMin and the leading non-zero of the significand is not assumed.

### Specials
- Infinity ignores sign of significand. Sign trit S carries the sign of infinity.
- For NaN, payload may be carried in the significand. Quiet vs signaling is encoded by the type code.

### Rounding model (informal sketch)
- The type code for reals encodes the rounding direction that produced the value:
  - `0-` rounded down,
  - `00` exact,
  - `0+` rounded up.
- TBD: mapping to IEEE-754 rounding modes and exception flags.

## Encoding order
The textual diagram above lists fields from most significant trit (left) to least significant trit (right). Hardware/bit-packing order for software is left open; the reference C++ header below packs to a 64-bit host word with base-3 digits (offset encoding 0..2).

## Status
This is a living draft. Open questions to nail down:
- Payload width and rules for NaN.
- Total vs compatible order for comparisons.
- Canonicalization rules for subnormals and zeros.

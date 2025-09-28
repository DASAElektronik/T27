# Ternary6: Mini floating-point format (draft)

A tiny 6-trit format used for test vectors and bring-up.

## Field layout

```
[ T1 T0 ] [ S ] [ E1 E0 ] [ F0 ]
  type     sign   exponent   significand
```

- Types: same coding as Ternary27 (`++` Inf, `+0` qNaN, `+-` sNaN, `0-` real down, `00` real exact, `0+` real up).
- Exponent range: small, e.g., [-4, +4] while drafting.
- Significand: 1 trit.

## Why this exists
- Easy to brute-force test (3^6 = 729 states).
- Good for FIFOs, shims and HDL stubs while the large format is still in flux.

## Test suggestions
- Exhaustive decode/encode round-trips for all 729 states.
- Cross-check special values (NaN classes, infinities).

<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Glossary & Notation

- **Balanced trit**: digit in {−1, 0, +1}; symbols: `N ≡ −1`, `Z ≡ 0`, `P ≡ +1`.
- **Little‑endian vector**: index `i` stores coefficient of `3^i`.
- **`ms(x)`**: index of the most significant non‑zero trit of vector `x`; if `x=0`, undefined.
- **`deg(x)`**: degree of `x`, i.e. `ms(x)`, with convention `deg(0) = −∞` (implemented as `-1` sentinel).
- **`sign(x)`**: sign of the most significant trit; `0` if `x=0`.
- **`canon(x)`**: canonicalization by stripping leading zeros (does not change the value).
- **`|x|` (magnitude)**: absolute value w.r.t. vector sign (`sign(x)`), implemented via negation if needed.
- **Truncation‑toward‑zero**: remainder `r` satisfies `sign(r) ∈ {0, sign(a)}` and `|r| < |b|`.
- **Shift `x << i`**: multiply by `3^i` (prepend `i` zeros in little‑endian representation).
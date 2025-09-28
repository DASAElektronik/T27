# Ternary Long Division (balanced trits, trunc‑toward‑zero)

*This page describes the algorithm. For invariants and termination, see
**[Proof sketches](proof_long_div.md)**.*

**Goal.** Compute `a = q·b + r` with `sign(r) = sign(a)` (or zero) and `|r| < |b|`.
Numbers are represented in **balanced trits** `{-1,0,+1}` little‑endian.

## Algorithm sketch
1. **Normalize divisor.** If the most‑significant trit (MS) of `b` is `−1`, negate `b` and remember a sign flip for `q`.
   After this, `lead(b) = +1`.
2. **Greedy elimination.** While `deg(R) ≥ deg(B)`, let `i = ms(R) − ms(B)` and `c = sign(R[ms(R)]) ∈ {−1,+1}`.
   Update `R ← R − c·(B << i)` and `Q ← Q + c·3^i`; then canonicalize `R`.
3. **Restore sign.** If we flipped the divisor in (1), negate `Q`.
4. **Normalize to truncation semantics.**
   If `sign(r) != sign(a)` or `|r| ≥ |b|`, adjust using `q ← q − 1, r ← r + b` or `q ← q + 1, r ← r − b`.
5. **Canonicalize.** Strip leading zeros after each step to keep degrees bounded.

**See also:** [Proof sketches](proof_long_div.md) for invariants, termination and uniqueness.
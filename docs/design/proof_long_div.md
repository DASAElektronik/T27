<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Shifted-divisor division: invariant and termination

The 2025 greedy leading-trit algorithm is replaced. Its proposed decreasing variant
was false: 5/4 cycles through remainders 5, -7, 5. Iteration caps and int64 fallback
do not establish a general ternary division algorithm.

## Preconditions and initialization

Normalize and validate inputs; reject b=0. Handle a=0 immediately. Work on positive
magnitudes A=abs(a), B=abs(b). For a positive balanced number with n significant trits:

`(3^(n-1)+1)/2 <= A <= (3^n-1)/2`.

The leading +1 outweighs the possible negative sum of lower digits. If n>=m,
choose k=n-m+1 and D=B*3^k. Then D >= (3^n+3^k)/2 > A.
If n<m, choose k=0; the same digit-range bounds give B>A. Thus initially R=A<D,
Q=0 and A=Q*B+R.

## One position

At position i use D=B*3^i. At entry `0<=R<3D`. Subtract D while R>=D, at most twice.
Each subtraction updates Q by +3^i, so A=Q*B+R is preserved. Subtraction only occurs
when its exact result is nonnegative. At exit `0<=R<D`.

For the next position D'=D/3, therefore the next entry has `0<=R<3D'`. This proves
the inner bound inductively. The code explicitly checks the postcondition after the
two possible subtractions and raises logic_error on an internal invariant violation.
The counter is never used to trigger a fallback.

## Termination

The outer position visits exactly k,k-1,...,0. The number of positions is finite
and decreases by one each iteration. Each contains at most two subtractions.
At exit D=B, hence `A=Q*B+R`, `0<=R<B`. The quotient is built using balanced-vector
addition of unit powers; a radix position with mathematical digit 2 is normalized
by that addition, never stored as an invalid Trit.

## Signed result and uniqueness

Negate Q iff the input signs differ; negate R iff a<0. Reconstruction follows
algebraically and sign(r) follows a. These are exactly the truncation-toward-zero
conditions. If two such pairs existed, the corresponding nonnegative-magnitude
remainders would differ by a multiple of B of magnitude less than B; therefore they
are equal, and so are the quotients.

## Cost and limits

There are at most 2*(k+1) subtractions. Shifted vectors, comparisons and additions
operate on O(n+m) trits, giving O((k+1)*(n+m)) time for this straightforward reference
and O(n+m) peak auxiliary space. No host-integer width bound is imposed. Allocation
limits still apply. This is a mathematical argument tied to the implementation,
not a machine-checked formal proof or a claim of optimal performance.

Tests: exhaustive signed small integers, deterministic 5/4 regression, wide known
quotient/remainder constructions, and Python arbitrary-precision oracle cases.

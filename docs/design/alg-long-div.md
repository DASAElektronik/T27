<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Exact balanced-ternary long division

1. Validate/canonicalize; reject b=0, handle a=0.
2. Use positive magnitudes A and B. Choose an initial shifted B strictly greater than A.
3. Visit shifts from high to low. At each shift subtract the trial at most twice,
   adding the corresponding unit power to the balanced quotient for each subtraction.
4. After the last shift restore signs: q follows a/b, r follows a.

The representation remains balanced-trit vectors throughout. There is no machine
integer conversion, greedy oscillation, iteration fallback, or approximate quotient.
See [proof_long_div.md](proof_long_div.md) and [numeric-contract.md](numeric-contract.md).

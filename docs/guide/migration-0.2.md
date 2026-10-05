<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Migration from the uploaded 0.1.1 snapshot

This revision intentionally corrects inconsistent behavior. Preserve an older
commit/snapshot before migrating and review the changes below. The repository
keeps src/t27-core and examples/bench, with a new root CMake build.

| Area | Old behavior | Candidate behavior |
| --- | --- | --- |
| mul27 overflow | Cyclic modulo 3^27-1 | Symmetric modulo 3^27, matching add/sub/shl |
| shr / shr27 | Repeated leading trit | Same zero-filled trit shift as lshr |
| Right shift flag | overflow for dropped trits | inexact for dropped trits, overflow=false |
| util parse/print | LSB-first | MSB-first; explicit *_lsb functions preserve old order |
| Null vector results | [] or [Z] | Always [Z] |
| Invalid/empty text | Ignored characters, empty accepted | invalid_argument |
| Negative shift/exponent | No-op | invalid_argument |
| from_bt out of range | Undefined signed overflow | overflow_error |
| Division | Oscillating greedy path + int64 fallback | Bounded shifted-divisor arithmetic on vectors |
| DivRR27 | No inexact member | inexact appended |
| Utility converters | Header-only, __int128 | Delegate to checked library implementation |
| Build | CMake + divergent handwritten VS projects | CMake as single source; VS Open Folder |
| Assertions | Some removed by NDEBUG | Tests use always-active CHECK |

The primitive Trit functions which now reject invalid enum values may throw and
no longer promise noexcept. Existing CMake strict flags remain accepted for callers.
Code using internal __t27_*/__v0_* helpers must use public APIs; these reserved-name
implementation details have been removed.

The candidate is numbered 0.2.0 because this is a behavior-changing pre-1.0 revision.
No public tag or release is created by this integration. New CI builds/tests,
checks installation, then creates a draft release on a matching tag; actual hosted
execution still needs verification after repository integration.

# Contributing to Ternary Computer (T27)

Thanks for considering a contribution! By contributing you agree to license
your work under the project's licenses and to the Patent Pledge.

## Quick Start
1. Fork the repo and create a topic branch.
2. Follow style guidelines and add tests where sensible.
3. Ensure `SPDX-License-Identifier` headers are present (see below).
4. Run linters/tests; update docs if behavior changes.
5. Open a PR with a clear description and checklist.

## Developer Certificate of Origin (DCO)
Add a Signed-off-by line to your commits:
```
Signed-off-by: Your Name <you@example.com>
```
This certifies that you have the right to submit the work under the project's licenses.

## Licensing Layout
- Code/HDL ⇒ Apache-2.0
- Hardware design files ⇒ CERN-OHL-P-2.0
- Documentation/media ⇒ CC-BY-4.0

## SPDX Headers (copy-paste)
For code (C/C++/Verilog/VHDL/Python, etc.):
```c
// SPDX-License-Identifier: Apache-2.0
```
For hardware design sources (schematics/PCB where supported by the tool, or in a README next to the files):
```
SPDX-License-Identifier: CERN-OHL-P-2.0
```
For docs/media where a header is possible:
```
SPDX-License-Identifier: CC-BY-4.0
```

## Pull Request Checklist
- [ ] Unit/logic tests updated or added (if applicable)
- [ ] Documentation updated (if applicable)
- [ ] SPDX headers present
- [ ] No inclusion of non-redistributable assets
- [ ] Passes CI

## Governance
Maintainers may merge PRs after review. Disagreements are resolved by
seeking consensus; failing that, maintainers decide.

## Security
Please see SECURITY.md for how to report vulnerabilities.

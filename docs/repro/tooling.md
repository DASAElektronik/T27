<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Tooling and versions

Minimum core requirements: CMake 3.20, C++20 compiler, Python 3.9 for the oracle.
Visual Studio presets target VS2022 x64. Ninja is needed for the Ninja preset.

The local candidate was checked with GCC 13.3.0, CMake 3.31.6, Ninja 1.11.1,
Python 3.12.14, clang-format 18.1.8 and Doxygen 1.18.0.

Documentation direct dependencies are pinned in `docs/requirements.txt`:
MkDocs 1.6.1 and Material 9.7.6. Transitive dependency resolution can still change.
CI records the runner/tool versions in its logs; it is not a bit-reproducible
container image. Doxygen is supplied by the runner distribution.

```sh
python -m pip install -r docs/requirements.txt
python tools/build_docs.py
```

For API only: configure CMake with `-DT27_BUILD_DOCS=ON`, then build target `docs`.
Doxygen writes to `build/doxygen/`; the combined-site helper stages its HTML under
ignored `docs/api/` and builds `site/`.

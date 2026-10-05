<!-- SPDX-License-Identifier: CC-BY-4.0 -->
# Quickstart

Requires CMake 3.20+, a C++20 compiler and Python 3.9+ for the independent oracle.

```sh
git clone https://github.com/DASAElektronik/T27.git
cd T27
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DT27_BUILD_TESTS=ON
cmake --build build --config Debug --parallel 2
ctest --test-dir build -C Debug --output-on-failure
```

Until the integration PR is merged, check out its branch to use these commands.
All six test groups, including `integer_oracle`, must run. CMake warns if Python
is missing; five passing groups do not satisfy the documented release gate.

## Visual Studio 2022

Open the repository **as a folder**, select the `vs2022-debug` CMake preset.
The old handwritten solution/project files have been removed.
From a VS2022 Developer PowerShell:

```powershell
cmake --preset vs2022-debug
cmake --build --preset build
ctest --preset ctest
cmake --build --preset release
ctest --preset ctest-release
```

## Documentation

Install Doxygen on PATH, then:

```sh
python -m pip install -r docs/requirements.txt
python tools/build_docs.py
```

Open `site/index.html` for the combined site, or run `python -m mkdocs serve`
after building the API. Doxygen-generated files are never committed.

Read [migration](migration-0.2.md) before replacing an older core and see
[examples](examples.md) for library use.

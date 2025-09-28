# Quickstart

Welcome to **T27 — Ternary Computer (Community Edition)**.

## Clone and open

```bash
git clone https://github.com/DASAElektronik/T27.git
cd T27
```

Open the folder in **Visual Studio 2022** (or open `T27.sln` once created).

## Build the docs locally

```bash
pip install mkdocs mkdocs-material
mkdocs serve
```
Then visit http://127.0.0.1:8000/ in your browser.

## Build the code (Windows, VS2022)

1. Create a solution **T27.sln** in the repo root.
2. Add a C++ Console App project under `src/` (e.g., `src/t27-core`).
3. Set Platform to **x64** and C++ language standard to **/std:c++20**.
4. Add your existing `.cpp`/`.h` files to the project (Right click project → Add → Existing Item…).

See also: [Contributing](contributing.md) and the [Roadmap](../roadmap.md).

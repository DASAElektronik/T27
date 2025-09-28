# Tooling & Versions

To keep results reproducible, record exact versions here.

## Suggested baseline
- Windows 11 (build X)
- Visual Studio 2022 17.xx
- CMake (if used), v3.xx
- Python 3.11+ (for docs)

## Docs build
```bash
pip install -r docs/requirements.txt
mkdocs build --strict
```

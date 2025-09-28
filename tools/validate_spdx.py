#!/usr/bin/env python3
import os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INCLUDE_EXT = {'.c','.h','.cpp','.hpp','.py','.v','.vh','.sv','.svh','.vhd','.md','.txt','.yml','.yaml','.cmake','.js','.ts','.rs','.go','.java'}
SKIP_DIRS = {'.git','build','out','dist','node_modules','vcpkg_installed','.venv','venv','.vs','.vscode','CMakeFiles','.github'}

missing = []
for base, dirs, files in os.walk(ROOT):
    dirs[:] = [d for d in dirs if d not in SKIP_DIRS and not d.startswith('.')]
    for fn in files:
        ext = os.path.splitext(fn)[1].lower()
        if ext in INCLUDE_EXT:
            path = os.path.join(base, fn)
            try:
                with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                    head = f.read(300)
                if 'SPDX-License-Identifier:' not in head:
                    missing.append(os.path.relpath(path, ROOT))
            except Exception as e:
                print(f"Warning: cannot read {path}: {e}")

if missing:
    print("Files missing SPDX headers (informational):")
    for m in missing:
        print(" -", m)
    # Do not fail the build yet; convert to failure later when repo stabilizes.
    sys.exit(0)
else:
    print("All checked files contain an SPDX header (or none to check).")

#!/usr/bin/env bash
set -euo pipefail
BUILD_DIR="${1:-build}"

if ! command -v lcov >/dev/null; then
  echo "lcov not found. Install with: sudo apt-get install -y lcov" >&2
  exit 1
fi
if ! command -v genhtml >/dev/null; then
  echo "genhtml not found. Install with: sudo apt-get install -y lcov" >&2
  exit 1
fi

cmake --build "$BUILD_DIR" --target coverage-lcov
xdg-open "$BUILD_DIR/coverage-html/index.html" >/dev/null 2>&1 || true
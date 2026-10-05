# SPDX-License-Identifier: Apache-2.0
"""Build one MkDocs site containing the generated Doxygen API reference."""
from pathlib import Path
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parent.parent
(root / "build/doxygen").mkdir(parents=True, exist_ok=True)
subprocess.run(["doxygen", "docs/Doxyfile"], cwd=root, check=True)
generated = root / "build/doxygen/html"
if not (generated / "index.html").is_file():
    raise SystemExit("Doxygen did not produce its HTML index")
api = root / "docs/api"
if api.exists():
    shutil.rmtree(api)
shutil.copytree(generated, api)
subprocess.run([sys.executable, "-m", "mkdocs", "build", "--strict"], cwd=root, check=True)
if not (root / "site/api/index.html").is_file():
    raise SystemExit("Combined site is missing its API reference")
print("Combined documentation: site/index.html and site/api/index.html")

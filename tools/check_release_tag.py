# SPDX-License-Identifier: Apache-2.0
import os
from pathlib import Path
import re
version=re.search(r'project\(t27 VERSION ([0-9.]+)',Path('CMakeLists.txt').read_text()).group(1)
if os.environ.get('GITHUB_REF_NAME')!='v'+version:
    raise SystemExit('Tag must match CMake version v'+version)

# SPDX-License-Identifier: Apache-2.0
"""Smoke-test an installed library with a separate CMake consumer."""
from pathlib import Path
import subprocess
import sys
import tempfile
build=Path(sys.argv[1]).resolve()
config=sys.argv[2] if len(sys.argv)>2 else 'Release'
with tempfile.TemporaryDirectory(prefix='t27-consumer-') as directory:
    root=Path(directory);prefix=root/'install';src=root/'consumer';src.mkdir()
    subprocess.run(['cmake','--install',str(build),'--config',config,'--prefix',str(prefix)],check=True)
    (src/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.20)\nproject(consumer LANGUAGES CXX)\nfind_package(t27 0.2.0 EXACT CONFIG REQUIRED)\nadd_executable(consumer main.cpp)\ntarget_link_libraries(consumer PRIVATE T27::t27)\nenable_testing()\nadd_test(NAME installed COMMAND consumer)\n')
    (src/'main.cpp').write_text('#include <t27/num/div.hpp>\n#include <t27/num/convert.hpp>\nint main(){using namespace t27::num;auto d=divmod(to_bt(5),to_bt(4));return from_bt(d.q)==1 && from_bt(d.r)==1?0:1;}\n')
    subprocess.run(['cmake','-S',str(src),'-B',str(root/'build'),'-DCMAKE_PREFIX_PATH='+str(prefix),'-DCMAKE_BUILD_TYPE='+config],check=True)
    subprocess.run(['cmake','--build',str(root/'build'),'--config',config],check=True)
    subprocess.run(['ctest','--test-dir',str(root/'build'),'-C',config,'--output-on-failure'],check=True)

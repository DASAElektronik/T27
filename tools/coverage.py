# SPDX-License-Identifier: Apache-2.0
import os
from pathlib import Path
import subprocess
import sys
mode,build,config,ctest,tool,aux,*binaries=sys.argv[1:]
build=Path(build).resolve()
def run(args,**kwargs):subprocess.run(list(map(str,args)),cwd=build,check=True,**kwargs)
env=os.environ.copy()
if mode=='llvm':
    profiles=build/'cov';profiles.mkdir(exist_ok=True)
    for p in profiles.glob('*.profraw'):p.unlink()
    env['LLVM_PROFILE_FILE']=str(profiles/'%p-%m.profraw')
run([ctest,'-C',config,'--output-on-failure'],env=env)
if mode=='llvm':
    raw=sorted(profiles.glob('*.profraw'))
    if not raw:raise SystemExit('No profiles generated')
    merged=profiles/'default.profdata'
    run([aux,'merge','-sparse',*raw,'-o',merged])
    args=[tool,'show',binaries[0],'-instr-profile='+str(merged),'-format=html','-output-dir='+str(build/'coverage-llvm'),'-ignore-filename-regex=(tests|header-tests)']
    for binary in binaries[1:]:args+=['-object',binary]
    run(args)
else:
    info=build/'coverage.info'
    run([tool,'--capture','--directory',build,'--output-file',info])
    run([tool,'--remove',info,'/usr/*','*/tests/*','*/header-tests/*','--output-file',info,'--ignore-errors','unused'])
    run([aux,info,'--output-directory',build/'coverage-html'])

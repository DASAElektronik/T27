# SPDX-License-Identifier: Apache-2.0
"""Manual GCC/Linux sensitivity experiment; never edits production source."""
import subprocess,json,sys
from pathlib import Path
root=Path(__file__).resolve().parents[3]
import os
os.chdir(root)
build=Path(sys.argv[1] if len(sys.argv)>1 else 'build/release').resolve()
tmp=build/'mutations'; tmp.mkdir(exist_ok=True)
original=(root/'experimental/cpu/src/cpu.cpp').read_text()
mutations={
'input-not-consumed':('io_.input.erase(io_.input.begin());', '// deliberately omit consumption'),
'eof-clobbers-data':('next.registers[i.rs1] = num::Tword27{}; // EOF: data destination unchanged.', 'next.registers[i.rs1] = num::Tword27{}; next.registers[i.rd] = num::Tword27{};'),
'wait-advances-pc':('return Stop::input_wait;', '{ ++state_.pc; return Stop::input_wait; }'),
'pop-clobbers-flags':('next.registers[i.rd] = value; // Restore without changing arithmetic flags.', 'write(value);'),
'call-saves-wrong-return':('num::to_bt(static_cast<std::int64_t>(next.pc))','num::to_bt(static_cast<std::int64_t>(state_.pc))'),
'fault-advances-pc':('state_.fault = fault;', 'state_.fault = fault; ++state_.pc;'),
}
results=[]
for name,(old,new) in mutations.items():
    assert original.count(old)==1
    source=tmp/(name+'.cpp'); binary=tmp/name; failure=tmp/(name+'.json')
    failure.unlink(missing_ok=True)
    source.write_text(original.replace(old,new))
    subprocess.run(['g++','-std=c++20','-O2','-Iinclude','-Iexperimental/cpu/include',str(source),'experimental/cpu/tests/cpu_trace_bridge.cpp',str(build/'libt27_cpu.a'),str(build/'libt27.a'),'-o',str(binary)],check=True)
    run=subprocess.run([sys.executable,'experimental/cpu/tests/cpu_oracle.py',str(binary),'--random-cases','0','--failure',str(failure)],capture_output=True,text=True)
    if run.returncode==0 or not failure.exists(): raise RuntimeError(name+': mutation not detected by state comparison: '+run.stderr)
    mismatch=json.loads(failure.read_text())
    differences=[k for k in mismatch['expected'] if mismatch['expected'][k]!=mismatch['actual'][k]]
    results.append(dict(mutation=name,replace_from=old,replace_to=new,detected=True,case=mismatch['case']['name'],step=mismatch['step'],differing_fields=differences))
(tmp/'report.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

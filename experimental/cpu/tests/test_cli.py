# SPDX-License-Identifier: Apache-2.0
"""Exercise the actual runner process, exit codes, diagnostics and checked-in programs."""
from pathlib import Path
import subprocess
import sys
import tempfile

runner, programs = Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve()
def run(args, status=0, out=(), err=()):
    result = subprocess.run([str(runner), *map(str, args)], capture_output=True, text=True, timeout=10)
    if result.returncode != status:
        raise AssertionError((args, result.returncode, result.stdout, result.stderr))
    for token in out:
        if token not in result.stdout:
            raise AssertionError((token, result.stdout))
    for token in err:
        if token not in result.stderr:
            raise AssertionError((token, result.stderr))

for name, registers in [('sum', ('R0=55\n', 'R5=55\n')), ('factorial', ('R0=720\n',)),
                        ('division', ('R2=-2\n', 'R3=-1\n')),
                        ('constants', ('R2=-3812798742493\n', 'overflow=1'))]:
    run([programs / (name + '.t27')], out=('stop=halted', *registers))
run(['--help'], out=('Usage:',))
run([], 2, out=('Usage:',))
with tempfile.TemporaryDirectory(prefix='t27-assembly-') as tmp:
    source = Path(tmp) / 'program with spaces.t27'
    def example(text, status=0, options=(), out=(), err=()):
        source.write_bytes(text.encode('ascii'))
        run([source, *options], status, out, err)
    example('HALT\r\n', out=('retired=1',))
    example('LI R9,1', 2, err=(':1:4:', 'R0 through R8'))
    example('NOP\nDIV R0,R1,R2', 3, err=(':2: fault=divide_by_zero',))
    example('again: JMP again', 4, ('--steps', '7'), out=('stop=step_limit pc=0 retired=7',))
    example('NOP', 3, ('--memory', '1'), err=('fetch_address',))
    example('JMP -2', 3, err=('branch_address',))
    example('LOAD R0,[R8-1]', 3, err=('data_address',))
    example('.word -1', 3, err=('illegal_instruction',))
    example('HALT\nHALT', 2, ('--memory', '1'), err=('smaller than program',))
    example(';empty', 2, err=('emits no words',))
    example('HALT\n' + ';' * 1048576, 2, err=('1 MiB',))
    example('HALT\n' + 'NOP\n' * 300, out=('stop=halted',)) # Default memory grows.
    for options in [('--steps', '0'), ('--steps', '-1'), ('--steps', '1x'), ('--steps',),
                    ('--steps', '18446744073709551616'), ('--memory', '1048577'),
                    ('--memory', '0'), ('--unknown', '1'), ('--steps', '1', '--steps', '2')]:
        example('HALT', 2, options)
    run([Path(tmp) / 'missing.t27'], 2, err=('cannot open',))
print('Assembler CLI: programs, input limits, fault diagnostics and exit codes PASS')

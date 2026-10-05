# SPDX-License-Identifier: Apache-2.0
"""Independent integer semantics for ISA v0.2; no imports from the C++ implementation.

Words are Python integers. Trit fields are extracted with modular arithmetic;
full-width products use arbitrary precision. This is a test model, not a second
production emulator or a cycle model.
"""
from collections import Counter

MODULUS = 3**27
LIMIT = (MODULUS-1)//2
IMM = (3**18-1)//2
# Used operand names, not the production codec's bit masks.
FORMS = {0: '', 1: '', 2: 'di', 3: 'da', 4: 'dab', 5: 'dab', 6: 'dab',
         7: 'dab', 8: 'dab', 9: 'dai', 10: 'abi', 11: 'i', 12: 'ai', 13: 'ai',
         -1: 'i', -2: '', -3: 'a', -4: 'd', -5: 'a', -6: 'a', -7: 'dai', -8: 'ai'}
FAULTS = {'fetch_address', 'illegal_instruction', 'divide_by_zero', 'data_address',
          'branch_address', 'stack_overflow', 'stack_underflow', 'io_endpoint'}

class Trap(Exception):
    pass

class Wait(Exception):
    pass

def encode(op, d=0, a=0, b=0, i=0):
    """Test generator only: independent numeric field assembly."""
    form = FORMS[op]
    parts = [(d-4 if 'd' in form else 0,3), (a-4 if 'a' in form else 0,5),
             (b-4 if 'b' in form else 0,7), (i if 'i' in form else 0,9)]
    if any(not 0 <= r < 9 for r in (d,a,b)) or not -IMM <= i <= IMM:
        raise ValueError('invalid generator operand')
    return op + sum(value*3**offset for value,offset in parts)

def decode(word):
    values = []
    for width in (3,2,2,2,18):
        radix = 3**width
        half = (radix-1)//2
        part = (word+half) % radix - half
        word = (word-part)//radix
        values.append(part)
    op,d,a,b,i = values
    if op not in FORMS or (op == -7 and d == a):
        raise Trap('illegal_instruction')
    form = FORMS[op]
    if any(value != 0 and name not in form for name,value in zip('dabi',(d,a,b,i))):
        raise Trap('illegal_instruction')
    return op, d+4 if 'd' in form else 0, a+4 if 'a' in form else 0, b+4 if 'b' in form else 0, i

class Model:
    def __init__(self, case):
        self.memory = case['image'][:] + [0]*(case['size']-len(case['image']))
        self.registers = case['registers'][:]
        self.pc, self.sp = case['entry'], case['end']
        self.begin, self.end = case['begin'], case['end']
        self.flags = [0,False,False]
        self.halted, self.fault = False,'none'
        config = case.get('io', {})
        self.io = dict(input=config.get('input', [])[:], output=[], closed=config.get('closed', False),
                       input_capacity=config.get('input_capacity',256), output_capacity=config.get('output_capacity',256))
        self.events = Counter()

    def state(self, reason='running', retired=0):
        return dict(reason=reason, retired=retired, pc=self.pc, sp=self.sp,
                    halted=self.halted, fault=self.fault, flags=self.flags[:],
                    registers=self.registers[:], memory=self.memory[:],
                    io={**self.io, 'input':self.io['input'][:], 'output':self.io['output'][:]}, host_result=[])

    def address(self, value, error):
        if not 0 <= value < len(self.memory):
            raise Trap(error)
        return value

    def push(self, value):
        if self.sp == self.begin:
            raise Trap('stack_overflow')
        self.sp -= 1
        self.memory[self.sp] = value

    def pop(self):
        if self.sp == self.end:
            raise Trap('stack_underflow')
        value = self.memory[self.sp]
        self.sp += 1
        return value

    def write(self, destination, exact, inexact=False):
        result = (exact+LIMIT) % MODULUS - LIMIT
        self.registers[destination] = result
        self.flags = [(result>0)-(result<0), abs(exact)>LIMIT, inexact]

    def execute(self):
        pc = self.address(self.pc, 'fetch_address')
        op,d,a,b,imm = decode(self.memory[pc])
        self.events['opcode:'+str(op)] += 1
        x,y = self.registers[a],self.registers[b]
        self.pc += 1
        if op in (-7,-8):
            if imm != 0: raise Trap('io_endpoint')
            if op == -7:
                if self.io['input']:
                    self.registers[d] = self.io['input'].pop(0)
                    self.registers[a] = 1
                    self.events['io:input'] += 1
                elif self.io['closed']:
                    self.registers[a] = 0
                    self.events['io:eof'] += 1
                else:
                    raise Wait('input_wait')
            else:
                if len(self.io['output']) == self.io['output_capacity']:
                    raise Wait('output_wait')
                self.io['output'].append(x)
                self.events['io:output'] += 1
        elif op == 0:
            pass
        elif op == 1:
            self.halted = True
            self.pc = pc
        elif op == 2:
            self.write(d,imm)
        elif op == 3:
            self.write(d,x)
        elif op in (4,5,6):
            self.write(d, x+y if op==4 else x-y if op==5 else x*y)
        elif op in (7,8):
            if y == 0:
                raise Trap('divide_by_zero')
            q = (abs(x)//abs(y)) * (-1 if (x<0)!=(y<0) else 1)
            r = x-q*y
            self.write(d,q if op==7 else r, op==7 and r!=0)
        elif op in (9,10):
            address = self.address(x+imm,'data_address')
            if op == 9:
                self.write(d,self.memory[address])
            else:
                self.memory[address] = y
        elif op in (11,12,13):
            taken = op==11 or (x==0 if op==12 else x!=0)
            if op != 11:
                self.events['branch:'+str(op)+':'+str(taken)] += 1
            if taken:
                self.pc = self.address(pc+1+imm,'branch_address')
        elif op in (-1,-5,-6):
            target = self.address(pc+1+imm if op==-1 else x,'branch_address')
            if op != -5:
                self.address(pc+1,'branch_address')
                self.push(pc+1)
            self.pc = target
        elif op == -3:
            self.push(x)
        elif op == -4:
            self.registers[d] = self.pop() # Flags preserved.
        elif op == -2:
            self.pc = self.address(self.pop(),'branch_address')

    def run(self, budget):
        retired = 0
        if self.fault != 'none': return self.state('fault')
        if self.halted: return self.state('halted')
        for _ in range(budget):
            before = self.state()
            try:
                self.execute()
            except (Trap,Wait) as fault:
                # Reference uses rollback; production stages changes before committing.
                for field in ('pc','sp','flags','registers','memory','halted','io'):
                    setattr(self, field, before[field])
                if isinstance(fault, Wait):
                    self.events['wait:'+str(fault)] += 1
                    return self.state(str(fault),retired)
                self.fault = str(fault)
                self.events['fault:'+self.fault] += 1
                return self.state('fault',retired)
            retired += 1
            if self.halted: return self.state('halted',retired)
        return self.state('step_limit',retired)

    def apply(self, action):
        op = action['op']
        result = []
        if op == 'run': return self.run(action['budget'])
        if op == 'feed':
            words = action['words']
            accepted = not self.io['closed'] and len(self.io['input'])+len(words) <= self.io['input_capacity']
            if accepted: self.io['input'].extend(words)
            result = [int(accepted)]
            self.events['host:feed:'+str(accepted)] += 1
        elif op == 'close': self.io['closed'] = True
        elif op == 'drain':
            result = self.io['output'][:]
            self.io['output'].clear()
        elif op == 'reset':
            self.pc, self.sp = action.get('entry',0), self.end
            self.registers = [0]*9
            self.flags = [0,False,False]
            self.halted, self.fault = False,'none'
        elif op == 'reset_io':
            self.io['input'].clear(); self.io['output'].clear(); self.io['closed'] = False
        else: raise ValueError('unknown host action')
        snapshot = self.state()
        snapshot['host_result'] = result
        return snapshot

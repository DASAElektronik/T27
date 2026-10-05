# SPDX-License-Identifier: Apache-2.0
"""Independent integer oracle. No third-party Python packages or T27 arithmetic."""
import random
import subprocess
import sys
M=3**27
MAX=(M-1)//2
def bt(n):
    if n==0:return '0'
    digits=[]
    while n:
        n,d=divmod(n,3)
        if d==2:n+=1;d=-1
        digits.append({-1:'-',0:'0',1:'+'}[d])
    return ''.join(reversed(digits))
def wrap(n):return (n+MAX)%M-MAX
def quot(a,b,mode):
    q=abs(a)//abs(b)*(1 if (a<0)==(b<0) else -1)
    r=a-q*b
    if mode=='floor':q=a//b
    elif mode=='ceil':q=-((-a)//b)
    elif mode=='euclid':q=(a-(a%abs(b)))//b
    elif mode in ('away','even'):
        twice=2*abs(r)
        if twice>abs(b) or (twice==abs(b) and (mode=='away' or q%2)):
            q+=1 if (a<0)==(b<0) else -1
    return q,a-q*b
cases=[]
def case(line,expected):cases.append((line,expected))
def pair(op,a,b):
    q,r=quot(a,b,op)
    case(f'{op} {bt(a)} {bt(b)}',f'{bt(q)} {bt(r)}')
# Exhaustive signs, zeros, exact and half-way results; independent native integers.
for a in range(-30,31):
    for b in range(-16,17):
        if b:
            for op in ['div','trunc','floor','ceil','euclid','away','even']:pair(op,a,b)
rng=random.Random(0x272026)  # fixed reproducible seed
# Deliberately exceed int64, int128 and the 27-trit word in dynamic operations.
for i in range(240):
    bits=rng.choice([1,10,43,64,128,256,512,768])
    a=rng.getrandbits(bits)*rng.choice([-1,1])
    b=(rng.getrandbits(rng.randint(1,bits)) or 1)*rng.choice([-1,1])
    for op in ['div','trunc','floor','ceil','euclid','away','even']:pair(op,a,b)
    for op,n in [('add',a+b),('sub',a-b),('mul',a*b)]:case(f'{op} {bt(a)} {bt(b)}',bt(n))
    case(f'cmp {bt(a)} {bt(b)}',str((a>b)-(a<b)))
    k=rng.randrange(0,100)
    q=(a+(3**k-1)//2)//3**k
    case(f'shl {bt(a)} {k}',bt(a*3**k))
    case(f'shr {bt(a)} {k}',bt(q))
    case(f'd3 {bt(a)} {k}',f'{bt(q)} {bt(a-q*3**k)}')
values=[-MAX,-MAX+1,-3**26,-3,-2,-1,0,1,2,3,3**26,MAX-1,MAX]
values += [rng.randint(-MAX,MAX) for _ in range(80)]
for a in values:
    for b in values[:13]:
        for op,n in [('wadd',a+b),('wsub',a-b),('wmul',a*b)]:
            case(f'{op} {bt(a)} {bt(b)}',f'{bt(wrap(n))} {int(abs(n)>MAX)} 0')
        if b:
            q,rem=quot(a,b,'trunc');case(f'wdiv {bt(a)} {bt(b)}',f'{bt(q)} {bt(rem)} 0 0 {int(rem!=0)}')
        else:case(f'wdiv {bt(a)} 0','0 0 1 0 0')
        for mode in ['trunc','floor','ceil','euclid','away','even']:
            if b:
                q,rem=quot(a,b,mode)
                case(f'w{mode} {bt(a)} {bt(b)}',f'{bt(wrap(q))} {bt(rem)} 0 {int(abs(q)>MAX)} {int(rem!=0)}')
            else:case(f'w{mode} {bt(a)} 0','0 0 1 0 0')

    for k in [0,1,2,13,26,27,28,100]:
        n=a*3**k;q=(a+(3**k-1)//2)//3**k
        case(f'wshl {bt(a)} {k}',f'{bt(wrap(n))} {int(abs(n)>MAX)} 0')
        case(f'wshr {bt(a)} {k}',f'{bt(q)} 0 {int(a!=q*3**k)}')
for n in [-2**63-1,-2**63,-2**63+1,-3**40,0,3**39,2**63-2,2**63-1,2**63,3**100]:
    case('int '+bt(n),str(n) if -2**63<=n<2**63 else 'OVERFLOW')
for op in ['div','trunc','floor','ceil','euclid','away','even']:case(f'{op} + 0','INVALID')
for op in ['shr','shl','d3','wshr','wshl']:case(f'{op} + -1','INVALID')
proc=subprocess.run([sys.argv[1]],input='\n'.join(x for x,_ in cases)+'\n',text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=120)
if proc.returncode:raise SystemExit(proc.stderr or f'bridge exit {proc.returncode}')
actual=proc.stdout.splitlines()
if len(actual)!=len(cases):raise SystemExit(f'{len(actual)} results for {len(cases)} cases')
for i,((line,want),got) in enumerate(zip(cases,actual)):
    if got!=want:raise SystemExit(f'case {i}: {line}\nexpected: {want}\nactual:   {got}')
print(f'Python integer oracle: {len(cases)} cases passed; seed=0x272026, up to 768-bit operands.')

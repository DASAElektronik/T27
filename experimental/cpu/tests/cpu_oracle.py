# SPDX-License-Identifier: Apache-2.0
"""Deterministic differential CPU tests and replayable architectural traces."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import random
import subprocess
import tempfile
from cpu_reference import Model, encode, decode, LIMIT, IMM, FORMS, FAULTS, Trap

def require(condition, message):
    if not condition:
        raise AssertionError(message)

def case(name, image, registers=None, size=None, entry=0, begin=None, end=None, budgets=None):
    size = max(16,len(image)+8) if size is None else size
    return dict(name=name, image=image, registers=registers or [0]*9, size=size, entry=entry,
                begin=len(image) if begin is None else begin, end=size if end is None else end,
                budgets=budgets if budgets is not None else [0]+[1]*36+[0,3])

def corpus(seed, count):
    rng = random.Random(seed)
    cases = []
    for op in FORMS:
        regs=[0,7,-3,0,2,0,0,0,0]
        image=[encode(op,d=3,a=1,b=2),encode(1)]
        if op in (9,10): regs[1]=8
        if op in (11,12,13): regs[1]=0
        if op == -1: image=[encode(-1,i=1),encode(1),encode(-2)]
        if op in (-5,-6): regs[1]=1
        if op in (-2,-4): image=[encode(-3,a=4),encode(op,d=3),encode(1)]
        cases.append(case('opcode-'+str(op),image,regs))
    bounds=(-LIMIT,-LIMIT+1,-3,-1,0,1,3,LIMIT-1,LIMIT)
    for op in (4,5,6,7,8):
        for x in bounds:
            for y in bounds:
                for dest in (1,2,3):
                    regs=[0,x,y,0,0,0,0,0,0]
                    cases.append(case(f'arith-{op}-{x}-{y}-{dest}',[encode(op,d=dest,a=1,b=2),encode(1)],regs,budgets=[1,0,1,1]))
    for op in (12,13):
        for x in (-1,0,1):
            for delta in (-IMM,-1,0,1,IMM):
                cases.append(case(f'branch-{op}-{x}-{delta}',[encode(op,a=1,i=delta),encode(1)], [0,x,0,0,0,0,0,0,0]))
    # One directed witness for each architectural fault and sticky stop behavior.
    cases.extend([
        case('fetch-fault',[encode(0)],size=1), case('illegal-opcode',[-13]),
        case('division-zero',[encode(7,d=0,a=1,b=2)]),
        case('negative-data',[encode(9,d=0,a=1,i=-1)]),
        case('negative-branch',[encode(11,i=-2)]),
        case('full-stack',[encode(-3,a=0)],size=1), case('empty-stack',[encode(-2)]),
        case('last-call',[encode(-1,i=-1)],size=1,begin=0),
        case('bad-return',[encode(-3,a=0),encode(-2)],[-1]+[0]*8),
        case('self-modify',[encode(10,a=8,b=0,i=1),encode(0),encode(0)], [encode(1)]+[0]*8),
    ])
    # Every unused trit position of every instruction must be zero.
    for op,form in FORMS.items():
        used=set(range(3))
        for operand,offset,width in (('d',3,2),('a',5,2),('b',7,2),('i',9,18)):
            if operand in form: used.update(range(offset,offset+width))
        for position in set(range(27))-used:
            cases.append(case(f'reserved-{op}-{position}',[encode(op,a=1 if op==-7 else 0)+3**position],budgets=[1,1,0]))
    # Real recursion encoded independently, no shared assembler.
    for n in range(11):
        image=[encode(2,d=0,i=n),encode(-1,i=1),encode(1),encode(12,a=0,i=7),
               encode(-3,a=0),encode(2,d=1,i=1),encode(5,d=0,a=0,b=1),encode(-1,i=-5),
               encode(-4,d=1),encode(6,d=0,a=0,b=1),encode(-2),encode(2,d=0,i=1),encode(-2)]
        for capacity in (0,max(0,2*n),2*n+1,2*n+2):
            cases.append(case(f'recursive-{n}-{capacity}',image,size=len(image)+capacity,budgets=[0]+[1]*90+[0,3]))
    # Nonzero flags survive POP, and source aliases use old values.
    for seed_op,x,y in ((4,LIMIT,1),(7,-7,3)):
        for op in (-1,-2,-3,-4,-5,-6):
            regs=[0,x,y,0,4,0,0,0,0]
            image=[encode(-3,a=4),encode(seed_op,d=0,a=1,b=2),encode(0),
                   encode(op,d=0,a=4),encode(1)]
            cases.append(case(f'flags-{seed_op}-{op}',image,regs))
    for n in range(count):
        regs=[rng.choice(bounds) if rng.randrange(2) else rng.randint(-LIMIT,LIMIT) for _ in range(9)]
        if n%4 == 0:
            image=[rng.randint(-LIMIT,LIMIT) for _ in range(8)]
        else:
            ops=(2,3,4,5,6,7,8) if n%4 == 1 else tuple(FORMS)
            image=[]
            for pc in range(24):
                op=rng.choice(ops)
                imm=rng.randint(-7,7)
                if op in (11,12,13,-1): imm=rng.randint(0,24)-pc-1
                if op==2: imm=rng.choice((-IMM,IMM,0,rng.randint(-30,30)))
                image.append(encode(op,d=rng.randrange(9),a=rng.randrange(9),b=rng.randrange(9),i=imm))
            image.append(encode(1))
            if n%4 == 2: regs=[rng.randrange(25) for _ in range(9)]
        size=len(image)+rng.randrange(0,17)
        begin=rng.randrange(size+1) if n%7==0 else len(image)
        # Mostly one-step runs, plus batching to verify retired counts and budget resumability.
        budgets=[0]+([1]*40 if n%3 else [rng.randrange(5) for _ in range(12)])+[0,1]
        cases.append(case(f'random-{seed}-{n}',image,regs,size=size,begin=begin,budgets=budgets))
    cases.extend(io_corpus(seed,count))
    return cases

def io_corpus(seed,count):
    cases=[]
    echo=[encode(-7,d=0,a=1),encode(12,a=1,i=2),encode(-8,a=0),encode(11,i=-4),encode(1)]
    run=lambda n: dict(op='run',budget=n)
    feed=lambda *w: dict(op='feed',words=list(w))
    for capacity in (0,1,2):
        c=case('io-directed-'+str(capacity),echo)
        c['io']=dict(input_capacity=2,output_capacity=capacity)
        c['actions']=[run(0),run(20),feed(0,-LIMIT),feed(LIMIT),run(30),run(30),
                      dict(op='drain'),run(30),dict(op='drain'),run(30),feed(LIMIT),
                      dict(op='close'),feed(1),run(30),dict(op='drain'),run(30),dict(op='drain'),run(0),
                      dict(op='reset'),run(2),dict(op='reset_io'),dict(op='reset'),run(10),feed(7),run(30)]
        cases.append(c)
    for opcode in (-7,-8):
        for endpoint in (-IMM,-1,1,IMM):
            c=case(f'io-endpoint-{opcode}-{endpoint}',[encode(opcode,d=0,a=1,i=endpoint)])
            c['io']=dict(input_capacity=0,output_capacity=0)
            cases.append(c)
    cases.append(case('io-alias-illegal',[-1087]))
    for op,x,y in ((4,LIMIT,1),(7,-7,3)):
        c=case('io-flags-'+str(op),[encode(op,d=2,a=3,b=4),encode(-7,d=0,a=1),encode(-8,a=0),
                                 encode(-7,d=0,a=1),encode(1)], [0,0,0,x,y,0,0,0,0])
        c['io']=dict(input=[LIMIT],closed=True,input_capacity=1,output_capacity=1)
        cases.append(c)
    # Host schedules exercise pausing, retries, queue lifecycle and rejection independently of instruction budgets.
    rng=random.Random(seed ^ 0x1027)
    for n in range(count):
        d,a=rng.sample(range(9),2)
        program=[encode(-7,d=d,a=a),encode(12,a=a,i=2),encode(-8,a=d),encode(11,i=-4),encode(1)]
        if n%11==0: program[2]=encode(-8,a=d,i=rng.choice((-1,1)))
        if n%13==0: program[0]=encode(-7,d=d,a=d) # Noncanonical destination alias.
        cap=rng.randrange(5)
        c=case(f'io-random-{seed}-{n}',program)
        c['io']=dict(input_capacity=cap,output_capacity=rng.randrange(5),
                     input=[rng.choice((-LIMIT,0,LIMIT)) for _ in range(rng.randrange(cap+1))],closed=bool(n%3==0))
        c['actions']=[]
        for step in range(60):
            choice=rng.randrange(100)
            action=(run(rng.randrange(12)) if choice<55 else
                    feed(*(rng.choice((-LIMIT,0,LIMIT,rng.randint(-20,20))) for _ in range(rng.randrange(4)))) if choice<75 else
                    dict(op='drain') if choice<87 else dict(op='close') if choice<92 else
                    dict(op='reset') if choice<96 else dict(op='reset_io'))
            c['actions'].append(action)
        cases.append(c)
    return cases

def self_check():
    # Hand-calculated independent witnesses validate the oracle before trusting comparisons.
    for instruction,value in [((2,0,0,0,5),98309),((3,8,0,0,0),-861),
                              ((4,4,8,0,0),-7772),((-1,0,0,0,5),98414),((-4,8,0,0,0),104)]:
        require(encode(*instruction)==value,'reference encoding vector')
        require(decode(value)==instruction,'reference decoding vector')
    m=Model(case('self',[encode(6,d=0,a=1,b=2),encode(1)],[0,LIMIT,2,0,0,0,0,0,0]))
    require(m.run(1)['registers'][0]==-1 and m.flags==[-1,True,False],'reference wrap')
    m=Model(case('self',[encode(7,d=0,a=1,b=2)],[0,-7,3,0,0,0,0,0,0]))
    require(m.run(1)['registers'][0]==-2 and m.flags==[-1,False,True],'reference truncation')
    m=Model(case('self',[encode(-3,a=0),encode(-2)],[-1]+[0]*8))
    m.run(1); before=m.state(); after=m.run(1)
    require(after['fault']=='branch_address' and after['sp']==before['sp'] and after['memory']==before['memory'],'reference atomic return')

def actions(c):
    return c.get('actions', [dict(op='run',budget=n) for n in c['budgets']])

def protocol(cases):
    lines=[str(len(cases))]
    for c in cases:
        steps=actions(c)
        lines.append(' '.join(map(str,[c['size'],len(c['image']),c['entry'],c['begin'],c['end'],len(steps)])))
        lines.extend(' '.join(map(str,c[key])) for key in ('registers','image'))
        io=c.get('io',{})
        words=io.get('input',[])
        lines.append(' '.join(map(str,[io.get('input_capacity',256),io.get('output_capacity',256),int(io.get('closed',False)),len(words),*words])))
        for a in steps:
            op=a['op']
            fields=([0,a['budget']] if op=='run' else [1,len(a['words']),*a['words']] if op=='feed' else
                    [2] if op=='close' else [3] if op=='drain' else [4,a.get('entry',0)] if op=='reset' else [5])
            lines.append(' '.join(map(str,fields)))
    return '\n'.join(lines)+'\n'

def compare(bridge,cases,timeout,failure,trace=None):
    events=Counter(); snapshots=0; retired=0
    # Disk-backed stdout avoids pipe deadlocks and unbounded in-memory trace capture.
    with tempfile.TemporaryFile(mode='w+',encoding='utf-8') as output:
        run=subprocess.run([str(bridge)],input=protocol(cases),text=True,stdout=output,
                           stderr=subprocess.PIPE,timeout=timeout)
        require(run.returncode==0, f'bridge failed: {run.returncode}: {run.stderr}')
        output.seek(0)
        for c in cases:
            model=Model(c)
            if trace:
                trace.write(json.dumps(dict(protocol_version=2,case=c['name'],input=c))+'\n')
            for step,action in enumerate([None]+actions(c)):
                budget=action.get('budget') if action else None
                expected=model.state() if action is None else model.apply(action)
                line=output.readline()
                require(bool(line),f'missing bridge snapshot: {c["name"]}/{step}')
                actual=json.loads(line)
                if actual != expected:
                    record=dict(case=c,step=step,action=action,budget=budget,expected=expected,actual=actual)
                    failure.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
                    raise AssertionError(f'{c["name"]} snapshot {step}: mismatch; replay {failure}')
                if trace:
                    trace.write(json.dumps(dict(case=c['name'],step=step,action=action,budget=budget,state=actual))+'\n')
                snapshots+=1; retired+=actual['retired']
            events.update(model.events)
        require(not output.read().strip(),'extra bridge output')
    return dict(cases=len(cases),snapshots=snapshots,retired=retired,events=dict(sorted(events.items())))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('bridge',type=Path)
    parser.add_argument('--seed',type=int,default=270101)
    parser.add_argument('--random-cases',type=int,default=1000)
    parser.add_argument('--timeout',type=int,default=120)
    parser.add_argument('--replay',type=Path)
    parser.add_argument('--report',type=Path)
    parser.add_argument('--trace',type=Path)
    parser.add_argument('--failure',type=Path,default=Path('cpu-oracle-failure.json'))
    args=parser.parse_args()
    require(0<=args.random_cases<=10000,'random case limit 10000')
    self_check()
    cases=[json.loads(args.replay.read_text(encoding='utf-8'))['case']] if args.replay else corpus(args.seed,args.random_cases)
    handle=args.trace.open('w',encoding='utf-8') if args.trace else None
    try:
        report=compare(args.bridge.resolve(),cases,args.timeout,args.failure,handle)
    finally:
        if handle: handle.close()
    report.update(protocol_version=2,seed=args.seed,random_cases=0 if args.replay else 2*args.random_cases,
                  random_cases_per_family=0 if args.replay else args.random_cases,
                  corpus_sha256=hashlib.sha256(protocol(cases).encode()).hexdigest())
    if not args.replay:
        events=report['events']
        require(all(events.get('opcode:'+str(op),0)>0 for op in FORMS),'missing opcode coverage')
        require(all(events.get('fault:'+fault,0)>0 for fault in FAULTS),'missing fault coverage')
        require(all(events.get(event,0)>0 for event in ('io:input','io:output','io:eof','wait:input_wait','wait:output_wait','host:feed:True','host:feed:False')), 'missing I/O outcomes')
        require(all(events.get(f'branch:{op}:{taken}',0)>0 for op in (12,13) for taken in (False,True)), 'missing branch outcomes')
    if args.report: args.report.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(report,sort_keys=True))

if __name__=='__main__':
    main()

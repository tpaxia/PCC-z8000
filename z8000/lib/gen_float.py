#!/usr/bin/env python3
"""Generate ABI wrappers for the integer-only IEEE runtime.

Numeric arguments are immediate words; bare offsets pass an address,
*offset loads a pointer, and voffset loads a scalar stack argument.
"""
from pathlib import Path
lines=['! IEEE software runtime. Big-endian word arguments on stack.\n! Internal conversion helpers use word/pair/quad widths explicitly.\n\t.text\n']
def emit(name,core,args,n=4,out=True):
 lines.append('\t.globl\t'+name+'\n'+name+':\n\tpush\t@sp,r13\n\tld\tr13,sp\n\tsub\tsp,#8\n')
 for arg in reversed(args):
  if isinstance(arg,int):lines.append('\tpush\t@sp,#%d\n'%arg)
  elif arg.startswith('*'):lines.append('\tld\tr0,%s(r13)\n\tpush\t@sp,r0\n'%arg[1:])
  elif arg.startswith('v'):lines.append('\tld\tr0,%s(r13)\n\tpush\t@sp,r0\n'%arg[1:])
  else:lines.append('\tlda\tr0,%s(r13)\n\tpush\t@sp,r0\n'%arg)
 if out:lines.append('\tlda\tr0,-8(r13)\n\tpush\t@sp,r0\n')
 lines.append('\tcall\t_'+core+'\n\tadd\tsp,#%d\n'%((len(args)+int(out))*2))  # softfp.c is C: its names carry the C underscore
 if out:
  for k in range(n):lines.append('\tld\tr%d,%d(r13)\n'%(k,-8+2*k))
 lines.append('\tld\tsp,r13\n\tpop\tr13,@sp\n\tret\n')
for name,core in [('fadd','daddcore'),('fsub','dsubcore'),('fmul','dmulcore'),('fdiv','ddivcore')]:emit(name,core,['4','12'])
emit('fneg','dnegcore',['4'])
for i,name in enumerate(['feq','fne','flt','fle','fgt','fge']):emit(name,'dcompare',['4','12',i],out=False)
emit('ftod','ftodcore',['4'])
emit('dtof','dtofcore',['4'],n=2)
for name,width,uns in [('itod',1,0),('utod',1,1),('ltod',2,0),('ultod',2,1)]:emit(name,'itodcore',['4',width,uns])
for name,width,uns in [('dtoi',1,0),('dtou',1,1),('dtol',2,0),('dtoul',2,1)]:emit(name,'dtoicore',['4',width,uns],n=width)
lines.append('\t.globl\tfnegf\nfnegf:\n\tld\tr0,2(sp)\n\txor\tr0,#32768\n\tld\tr1,4(sp)\n\tret\n')
for i,op in enumerate(['add','sub','mul','div']):
 emit('af'+op,'dafcore',['*4','6',i,0])
 emit('af'+op+'f','dafcore',['*4','6',i,1],n=2)
 emit('f'+op+'f','dfopcore',['4','8',i,1],n=2)
for name,single in [('dfpost',0),('ffpost',1)]:emit(name,'postcore',['*4','v6',single],n=2 if single else 4)
lines.append('\t.data\n\t.globl\tfltused\nfltused:\n\t.word\t0\n')
Path(__file__).with_name('float.az8').write_text(''.join(lines))

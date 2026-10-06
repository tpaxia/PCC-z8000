#!/usr/bin/env python3
"""Compact PCC Z8002 C assembly using shared frames and local peepholes.

Reads stdin and writes stdout. Link output with lib/csv.az8. The frame layout
and C calling convention are unchanged. Intended for compiler-generated C,
not arbitrary hand-written assembly (C calls do not return condition codes).
"""
import re
import sys

regs=[4,5,6,7,10,11,12,14]
saves=''.join('\tld\t-%d(r13),r%d\n'%(2*(i+1),r) for i,r in enumerate(regs))
pro=re.compile(r'\tpush\t@sp,r13\n\tld\tr13,sp\n\tsub\tsp,#(_F\d+)\n'+re.escape(saves))
epi=re.compile(r'(?:\tld\tr(?:4|5|6|7|10|11|12|14),-\d+\(r13\)\n)*\tld\tsp,r13\n\tpop\tr13,@sp\n\tret\n')
def compact(s):
 s,n=pro.subn(lambda m:'\tld\tr8,#'+m[1]+'\n\tcall\tcsv\n',s)
 if not n: return s
 s=epi.sub('\tjp\tcret\n',s)
 lines=s.splitlines()
 for turn in range(8):
  before=lines[:]
  # Remove unreachable instructions, retaining all labels and directives.
  result=[];dead=False
  for line in lines:
   text=line.strip()
   if not text or text.startswith('!'):result.append(line);continue
   if text.endswith(':') or text.startswith('.') or '=' in text:dead=False
   elif dead:continue
   result.append(line)
   if re.fullmatch(r'(jr|jp)\s+\.?[A-Za-z_][\w.]*',text) or text=='ret':dead=True
  lines=result
  # Redirect branches through labels that do nothing but branch again.
  targets={}
  for i,line in enumerate(lines[:-1]):
   if re.fullmatch(r'\.L\d+:',line.strip()):
    j=i+1
    while j<len(lines) and (not lines[j].strip() or lines[j].lstrip().startswith('!')):j+=1
    if j<len(lines):
     m=re.fullmatch(r'\s*(jr|jp)\s+(\.L\d+)\s*',lines[j])
     if m:targets[line.strip()[:-1]]=m[2]
  result=[]
  for line in lines:
   if re.match(r'\s*(jr|jp)\s',line):
    m=re.search(r'(\.L\d+)$',line)
    if m and m[1] in targets and targets[m[1]]!=m[1]:line=line[:m.start()]+targets[m[1]]
   result.append(line)
  lines=result;result=[]
  for i,line in enumerate(lines):
   m=re.fullmatch(r'\s*(jr|jp)\s+(\.L\d+)\s*',line)
   if m:
    j=i+1
    while j<len(lines) and (not lines[j].strip() or lines[j].strip().endswith(':') or lines[j].lstrip().startswith('!')):
     if lines[j].strip()==m[2]+':':break
     j+=1
    if j<len(lines) and lines[j].strip()==m[2]+':':continue
   if re.fullmatch(r'\s*ld\s+(r\d+),\1\s*',line):continue
   result.append(line)
  lines=result
  if lines==before:break
 s='\n'.join(lines)+'\n'
 s=re.sub(r'\tld\t(r\d+),#0\n',r'\tclr\t\1\n',s)
 # Stack cleanup does not carry a C expression result in the flags.
 s=re.sub(r'\tadd\tsp,#([1-9]|1[0-6])\n',r'\tinc\tsp,#\1\n',s)
 s=re.sub(r'\tsub\tsp,#([1-9]|1[0-6])\n',r'\tdec\tsp,#\1\n',s)
 def pairload(m):
  a,off,base,b,off2,base2=map(int,m.groups())
  if a%2==0 and b==a+1 and off2==off+2 and base==base2 and base!=a:
   return '\tldl\trr%d,%d(r%d)\n'%(a,off,base)
  return m[0]
 s=re.sub(r'\tld\tr(\d+),(-?\d+)\(r(\d+)\)\n\tld\tr(\d+),(-?\d+)\(r(\d+)\)\n',pairload,s)
 def pairreg(m):
  a,b,c,d=map(int,m.groups())
  return '\tldl\trr%d,rr%d\n'%(a,b) if a%2==b%2==0 and c==a+1 and d==b+1 else m[0]
 s=re.sub(r'\tld\tr(\d+),r(\d+)\n\tld\tr(\d+),r(\d+)\n',pairreg,s)
 def pairstore(m):
  off,base,a,off2,base2,b=map(int,m.groups())
  if a%2==0 and b==a+1 and off2==off+2 and base==base2:
   return '\tldl\t%d(r%d),rr%d\n'%(off,base,a)
  return m[0]
 s=re.sub(r'\tld\t(-?\d+)\(r(\d+)\),r(\d+)\n\tld\t(-?\d+)\(r(\d+)\),r(\d+)\n',pairstore,s)
 return s

if __name__ == '__main__':
    sys.stdout.write(compact(sys.stdin.read()))

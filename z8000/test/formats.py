#!/usr/bin/env python3
"""Reject historical and unsupported images in the standalone s.out runner."""
from pathlib import Path
import struct
import subprocess
import tempfile
HERE = Path(__file__).resolve().parent
base = (HERE / 'hello.sout').read_bytes()
assert subprocess.run([str(HERE/'run_emu'),str(HERE/'hello.sout'),'-e','42'],capture_output=True).returncode == 0
fixtures = {}
for magic in (0o405,0o407,0o410,0o411):
    fixtures['old16-%o'%magic] = struct.pack('>8H',magic,2,0,0,0,0,0,0) + bytes.fromhex('9e08') + bytes(32)
    fixtures['old32-%o'%magic] = struct.pack('>8I',magic,2,0,0,0,0,0,0) + bytes.fromhex('9e08') + bytes(16)
for name,offset,value in [('split',0,0xe711),('seg',0,0xe607),('descriptor',10,32),
                          ('flags',18,0),('reloc',14,2),('entry',16,1),('segment',24,1),
                          ('overflow',32,65535),('attrs',34,8)]:
    data=bytearray(base);struct.pack_into('>H',data,offset,value);fixtures[name]=data
fixtures['short']=base[:39]
fixtures['truncated']=base[:-1]
with tempfile.TemporaryDirectory(prefix='pcc-sout-') as work:
    for name,data in fixtures.items():
        p=Path(work)/name;p.write_bytes(data)
        result=subprocess.run([str(HERE/'run_emu'),str(p)],capture_output=True)
        assert result.returncode and b's.out' in result.stderr,(name,result.stderr)
print('PASS standalone loader: %d obsolete/malformed/unsupported images rejected'%len(fixtures))

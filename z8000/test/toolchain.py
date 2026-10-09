"""s.out tools shared with the Unix port (host and native use the same C)."""
import sys
sys.dont_write_bytecode = True
import os
from pathlib import Path
ROOT = Path(os.environ.get('SOUT_ROOT', Path(__file__).resolve().parents[3])).resolve()
AS = ROOT / 'tests/build/asz8k-host/asz8k'
LD = ROOT / 'tests/build/ldz8-host/ldz8'

def setup_commands():
    if not (ROOT / 'tools/asz8k/Makefile').exists():
        raise RuntimeError('Set SOUT_ROOT to the z8000_unix checkout containing the shared s.out tools')
    return [['make', '-C', ROOT / 'tools/asz8k'],
            ['make', '-C', ROOT / 'tools/ldz8']]

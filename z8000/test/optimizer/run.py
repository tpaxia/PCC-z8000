#!/usr/bin/env python3
"""Optimizer equivalence, bounded-memory fallback, and malformed-input tests."""
from pathlib import Path
import importlib.util
import subprocess
import sys
sys.dont_write_bytecode = True
TARGET = Path(__file__).resolve().parents[2]
TEST = TARGET / 'test'
WORK = Path(__file__).resolve().parent / 'build'
WORK.mkdir(exist_ok=True)
subprocess.run(['make', '-C', str(TEST), '../oz8', 'run_emu', 'crt0.b', 'csv.b', 'exit.b'], check=True)
spec = importlib.util.spec_from_file_location('reference', TARGET / 'c2z8.py')
reference = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reference)
entry = '\t.text\n\t.globl _main\n_main:\n\tld\tr8,#16\n\tcall\tcsv\n'
finish = '\tld\tr0,#0\n\tjp\tcret\n'
fixtures = {
    'branches': entry + '\tjr\t.L1\n\tld\tr0,#99\n.L1:\n! comment\n\tjr\t.L2\n.L2:\n\tld\tr1,r1\n' + finish,
    'pairs': entry + '\tld\tr0,-20(r13)\n\tld\tr1,-18(r13)\n\tld\tr4,r6\n\tld\tr5,r7\n\tld\t-24(r13),r4\n\tld\t-22(r13),r5\n\tadd\tsp,#4\n\tsub\tsp,#4\n' + finish,
    'load-alias': entry + '\tld\tr0,0(r0)\n\tld\tr1,2(r0)\n' + finish,
    'cycle': entry + '.L1:\n\tjr\t.L2\n.L2:\n\tjr\t.L1\n',
    'labels': entry + '\tjr\t.L2\n! retained\n.L1:\n.L2:\n' + finish,
    'directives': entry + '\tjr\t.L1\n\t.word 0\n.L1:\n' + finish,
    'large': entry + ('! ' + 'x' * 80 + '\n') * 2000 + finish,
    'hand-assembly': '\t.text\n\tld\tr0,r0\n\tret\n',
}
for name, source in fixtures.items():
    result = subprocess.run([str(TARGET / 'oz8')], input=source, text=True, capture_output=True, check=True)
    assert result.stdout == reference.compact(source), name
    (WORK / (name + '.az8')).write_text(result.stdout)
    print('PASS reference', name)

# More aliases than the jump map can hold: optimization must remain correct.
source = entry + '\tjr\t.L2999\n'
source += ''.join('.L%d:\n\tjr\t.L9999\n' % i for i in range(3000))
source += '.L9999:\n' + finish
result = subprocess.run([str(TARGET / 'oz8')], input=source, text=True, capture_output=True, check=True)
assert '.L2999:' in result.stdout
(WORK / 'bounded.az8').write_text(result.stdout)
subprocess.run([str(TARGET / 'az8/az8'), '-o', 'bounded.b', 'bounded.az8'], cwd=WORK, check=True)
subprocess.run(list(map(str, [TARGET / 'ldz8', '-x', TEST / 'crt0.b', '-R', '8', WORK / 'bounded.b',
                              TEST / 'csv.b', TEST / 'exit.b', '-o', WORK / 'bounded.out'])), check=True)
subprocess.run(list(map(str, [TEST / 'run_emu', WORK / 'bounded.out', '-e', '0'])), check=True)
print('PASS bounded-map execution')
result = subprocess.run([str(TARGET / 'oz8')], input='x'*1024+'\n', text=True, capture_output=True)
assert result.returncode and not result.stdout and 'line too long' in result.stderr
print('PASS overlong-line rejection')

# Available current generated corpora provide a broader byte-for-byte oracle.
count = 0
for path in sorted((TEST / 'ratchet/build').glob('*/*.az8')):
    source = path.read_text()
    if '\tcall\tcsv\n' not in source:
        continue
    result = subprocess.run([str(TARGET / 'oz8')], input=source, text=True, capture_output=True, check=True)
    assert result.stdout == reference.compact(source), str(path)
    count += 1
print('PASS generated corpus:', count, 'files')

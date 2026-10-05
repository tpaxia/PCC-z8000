#!/usr/bin/env python3
"""Check the integer-only binary64 adder against host IEEE arithmetic."""
import ctypes
import math
from pathlib import Path
import random
import struct
import subprocess
import tempfile


def main():
    source = Path(__file__).resolve().parents[2] / "lib" / "softfp.c"
    with tempfile.TemporaryDirectory(prefix="pcc-softfp-") as directory:
        library = Path(directory) / "softfp.so"
        subprocess.run(["cc", "-shared", "-fPIC", "-w", "-Wno-implicit-int",
                        "-Wno-implicit-function-declaration", "-Wno-return-mismatch",
                        str(source), "-o", str(library)], check=True)
        lib = ctypes.CDLL(str(library))
        words = ctypes.c_ushort * 4
        lib.daddcore.argtypes = [ctypes.POINTER(ctypes.c_ushort)] * 3
        lib.daddcore.restype = None
        rng = random.Random(1707)
        count = 0
        while count < 100000:
            a, b = rng.getrandbits(64), rng.getrandbits(64)
            # Finite operands; overflow is checked as infinity. NaN payloads
            # are checked separately in the target fixtures.
            if ((a >> 52) & 2047) == 2047 or ((b >> 52) & 2047) == 2047:
                continue
            aw = words(*(a >> shift & 65535 for shift in (48, 32, 16, 0)))
            bw = words(*(b >> shift & 65535 for shift in (48, 32, 16, 0)))
            out = words()
            lib.daddcore(out, aw, bw)
            result = sum(int(out[k]) << (48 - 16*k) for k in range(4))
            af = struct.unpack(">d", a.to_bytes(8, "big"))[0]
            bf = struct.unpack(">d", b.to_bytes(8, "big"))[0]
            expected = int.from_bytes(struct.pack(">d", af + bf), "big")
            if result != expected:
                raise AssertionError(f"{a:016x} + {b:016x}: "
                                     f"got {result:016x}, expected {expected:016x}")
            count += 1
        print(f"PASS softfp: {count} deterministic IEEE binary64 additions")

        def value(bits):
            return struct.unpack(">d", bits.to_bytes(8, "big"))[0]

        def bits(value):
            return int.from_bytes(struct.pack(">d", value), "big")

        def array(bits):
            return words(*(bits >> shift & 65535 for shift in (48, 32, 16, 0)))

        def result(out):
            return sum(int(out[k]) << (48 - 16*k) for k in range(4))

        edges = [0, 1 << 63, 1, 0xfffffffffffff, 0x10000000000000,
                 0x3ff0000000000000, 0xbff0000000000000,
                 0x7fefffffffffffff, 0x7ff0000000000000, 0xfff0000000000000,
                 0x7ff8000000000001]
        for name, operation in [("dsubcore", lambda a,b: a-b),
                                ("dmulcore", lambda a,b: a*b),
                                ("ddivcore", lambda a,b: a/b)]:
            function = getattr(lib, name)
            function.argtypes = [ctypes.POINTER(ctypes.c_ushort)] * 3
            pairs = [(a,b) for a in edges for b in edges]
            pairs += [(rng.getrandbits(64), rng.getrandbits(64)) for _ in range(20000)]
            checked = 0
            for a,b in pairs:
                af,bf = value(a),value(b)
                if name == "ddivcore" and bf == 0:
                    expected = math.nan if af == 0 or math.isnan(af) else math.copysign(math.inf, af * math.copysign(1, bf))
                else:
                    expected = operation(af,bf)
                out = words()
                function(out,array(a),array(b))
                got = result(out)
                if math.isnan(expected):
                    assert math.isnan(value(got)), (name,hex(a),hex(b),hex(got))
                else:
                    assert got == bits(expected), (name,hex(a),hex(b),hex(got),hex(bits(expected)))
                checked += 1
            print(f"PASS softfp: {checked} {name} IEEE checks")

        for name in ("ftodcore", "dtofcore"):
            function = getattr(lib,name)
            function.argtypes = [ctypes.POINTER(ctypes.c_ushort)] * 2
            for _ in range(20000):
                out = words()
                if name == "ftodcore":
                    a = rng.getrandbits(32)
                    expected = struct.unpack(">f",a.to_bytes(4,"big"))[0]
                    function(out,words(a >> 16,a & 65535,0,0))
                    got = result(out)
                    if math.isnan(expected):
                        assert math.isnan(value(got))
                    else:
                        assert got == bits(expected), (name,hex(a),hex(got))
                else:
                    a = rng.getrandbits(64)
                    function(out,array(a))
                    got = int(out[0]) << 16 | int(out[1])
                    if math.isnan(value(a)):
                        assert math.isnan(struct.unpack(">f",got.to_bytes(4,"big"))[0])
                    else:
                        try:
                            expected = int.from_bytes(struct.pack(">f",value(a)),"big")
                        except OverflowError:
                            expected = 0xff800000 if a >> 63 else 0x7f800000
                        assert got == expected, (name,hex(a),hex(got),hex(expected))
            print(f"PASS softfp: 20000 {name} conversion checks")

        for name in ("itodcore", "dtoicore"):
            getattr(lib,name).argtypes = [ctypes.POINTER(ctypes.c_ushort)] * 2 + [ctypes.c_int] * 2
        for width in (1,2):
            mask = (1 << (width * 16)) - 1
            for unsigned in (0,1):
                limit = 1 << (width * 16 - (not unsigned))
                values = [0,1,limit-1] + ([ -1,-limit ] if not unsigned else [])
                values += [rng.randrange(0 if unsigned else -limit,limit) for _ in range(2000)]
                for n in values:
                    encoded = words(n & mask,0,0,0) if width == 1 else words((n & mask) >> 16,n & 65535,0,0)
                    out = words()
                    lib.itodcore(out,encoded,width,unsigned)
                    assert result(out) == bits(float(n)), ("itod",width,unsigned,n,hex(result(out)))
                    lib.dtoicore(out,array(bits(float(n))),width,unsigned)
                    got = int(out[0]) if width == 1 else int(out[0]) << 16 | int(out[1])
                    assert got == n & mask, ("dtoi",width,unsigned,n,hex(got))
        lib.dcompare.argtypes = [ctypes.POINTER(ctypes.c_ushort)] * 2 + [ctypes.c_int]
        for a in edges:
            for b in edges:
                af,bf = value(a),value(b)
                expected = [af==bf, af!=bf, af<bf, af<=bf, af>bf, af>=bf]
                for op,answer in enumerate(expected):
                    assert bool(lib.dcompare(array(a),array(b),op)) == answer
        print("PASS softfp: integer conversion boundaries and ordered/unordered comparisons")


if __name__ == "__main__":
    main()

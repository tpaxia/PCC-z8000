# Makefile -- build and run PCC Z8000 end-to-end tests on the Z8002 emulator
#
# Usage: make          -- build everything and run all tests
#        make run_emu  -- build just the test driver
#        make clean    -- remove build artifacts

TOOLDIR   = ..
CZ8       = $(TOOLDIR)/cz8/cz8
AZ8       = $(TOOLDIR)/az8/az8
LDZ8      = $(TOOLDIR)/ldz8
OZ8       = $(TOOLDIR)/oz8

EMU_DIR   = ../../z8000_emu
# The emulator is a CMake project: the library headers are under lib/include
# (as <z8000/z8000.h>) and memory.h belongs to its front end in src.
EMU_INC   = -I$(EMU_DIR)/lib/include -I$(EMU_DIR)/src
EMU_LIB   = $(EMU_DIR)/build/lib/libz8000.a

CXX       = c++
CXXFLAGS  = -std=c++17 -Wall -Wextra $(EMU_INC) -O2

# Test programs and their expected R0 values
TESTS     = hello arith control switch bitfield shift larith \
            pcc_math pcc_cmp pcc_struct pcc_structret pcc_union pcc_ptr pcc_scope pcc_optim
hello_EXPECT      = 42
arith_EXPECT      = 120
control_EXPECT    = 0
switch_EXPECT     = 0
bitfield_EXPECT   = 0
shift_EXPECT      = 0
larith_EXPECT     = 0
pcc_math_EXPECT   = 0
pcc_cmp_EXPECT    = 0
pcc_struct_EXPECT = 0
pcc_structret_EXPECT = 0
pcc_union_EXPECT  = 0
pcc_ptr_EXPECT    = 0
pcc_scope_EXPECT  = 0
pcc_optim_EXPECT  = 0

# Build artifacts
BOUTS     = $(TESTS:%=%.bout)

.PHONY: all clean test regress regress-strict external compare68k ratchet knr parser-sync optimizer gate

all: test

# Broad K&R language/backend audit; rebuilds cases in regress/build.
regress:
	python3 regress/run.py

# Fail on any outstanding expected failures.
regress-strict:
	python3 regress/run.py --strict

# Selected upstream tests adapted for this compiler's K&R/16-bit model.
# Discovery failures are real failures; this target exits nonzero for any.
external:
	python3 external/run.py

# 68000 comparison probes.
compare68k:
	python3 compare68k/run.py

# No-regression ratchet over real K&R source (V7 kernel, shell, commands, libc
# and this toolchain's own sources). Compile-only; needs the V7 tree, see
# ratchet/README.md. A missing corpus fails rather than skips.
ratchet:
	python3 ratchet/run.py

# K&R reference-manual coverage probes; see knr/MATRIX.md for the result
# per rule. A probe recorded as passing must keep passing.
knr:
	python3 knr/run.py

# The checked-in cgram.c must be exactly what V7 yacc generates from cgram.y.
parser-sync:
	python3 ../cz8/regen_cgram.py --check

# Everything a compiler change must pass before it is accepted.
gate: parser-sync test regress-strict compare68k external ratchet knr optimizer

optimizer:
	python3 optimizer/run.py
	python3 regress/run.py --strict --compact

# The linker is K&R C; without this rule make's built-in rule compiles it with
# default flags, which a modern host compiler rejects.
$(LDZ8): $(TOOLDIR)/ldz8.c
	cc -O -w -Wno-implicit-int -Wno-implicit-function-declaration -Wno-return-mismatch -Wno-int-conversion -o $@ $<

$(OZ8): $(TOOLDIR)/oz8.c
	cc -std=gnu89 -O -w -Wno-return-mismatch -Wno-int-conversion -o $@ $<

# Build the emulator library if needed
$(EMU_LIB):
	cmake -S $(EMU_DIR) -B $(EMU_DIR)/build -DCMAKE_BUILD_TYPE=Release
	cmake --build $(EMU_DIR)/build --target z8000

# Build the test driver
run_emu: run_emu.cpp $(EMU_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(EMU_LIB)

# Assemble crt0
crt0.b: $(TOOLDIR)/crt0.az8 $(AZ8)
	$(AZ8) -o $@ $<

csv.b: $(TOOLDIR)/lib/csv.az8 $(AZ8)
	$(AZ8) -o $@ $<

# Assemble exit stub
exit.b: $(TOOLDIR)/lib/exit.az8 $(AZ8)
	$(AZ8) -o $@ $<

# Assemble arithmetic library (named liblong.b to avoid clash with arith.c test)
liblong.b: $(TOOLDIR)/lib/arith.az8 $(AZ8)
	$(AZ8) -o $@ $<

# Floating runtime used by the general regression tests and user programs.
libfloat.b: $(TOOLDIR)/lib/float.az8 $(AZ8)
	$(AZ8) -o $@ $<

softfp.az8: $(TOOLDIR)/lib/softfp.c $(CZ8)
	$(CZ8) < $< > $@

# Pattern rules for compiling and assembling test .c files
%.az8: %.c $(CZ8)
	$(CZ8) < $< > $@

%.b: %.az8 $(AZ8)
	$(AZ8) -o $@ $<

# Link: crt0 first (so _start is at origin), then test, then exit
# -R 8 = text origin at 0x0008 (after 6-byte PSAP reset vector area)
# -x = discard local symbols
%.bout: crt0.b %.b liblong.b libfloat.b softfp.b exit.b csv.b $(LDZ8)
	$(LDZ8) -x crt0.b -R 8 $*.b liblong.b libfloat.b softfp.b exit.b csv.b -o $@

# Run all tests
test: run_emu $(BOUTS)
	@pass=0; fail=0; \
	for pair in hello:$(hello_EXPECT) arith:$(arith_EXPECT) control:$(control_EXPECT) \
	            switch:$(switch_EXPECT) bitfield:$(bitfield_EXPECT) shift:$(shift_EXPECT) \
	            larith:$(larith_EXPECT) pcc_math:$(pcc_math_EXPECT) pcc_cmp:$(pcc_cmp_EXPECT) \
	            pcc_struct:$(pcc_struct_EXPECT) pcc_structret:$(pcc_structret_EXPECT) \
	            pcc_union:$(pcc_union_EXPECT) pcc_ptr:$(pcc_ptr_EXPECT) \
	            pcc_scope:$(pcc_scope_EXPECT) pcc_optim:$(pcc_optim_EXPECT); do \
		t=$${pair%%:*}; exp=$${pair##*:}; \
		if ./run_emu $$t.bout -e $$exp; then \
			pass=$$((pass + 1)); \
		else \
			fail=$$((fail + 1)); \
		fi; \
	done; \
	echo ""; \
	echo "Results: $$pass passed, $$fail failed out of $(words $(TESTS)) tests"; \
	[ $$fail -eq 0 ]

clean:
	rm -f run_emu *.az8 *.b *.bout

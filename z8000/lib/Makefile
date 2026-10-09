# Target runtime objects. Link these with compiler output and crt0.b.
CZ8 = ../cz8/cz8
SOUT_ROOT ?= ../../..
AZ8 = $(SOUT_ROOT)/tests/build/asz8k-host/asz8k

.PHONY: all
all: arith.b float.b softfp.b exit.b csv.b

$(CZ8):
	$(MAKE) -C ../cz8

$(AZ8):
	$(MAKE) -C $(SOUT_ROOT)/tools/asz8k

float.az8: gen_float.py
	python3 gen_float.py

softfp.az8: softfp.c $(CZ8)
	$(CZ8) < $< > $@

%.b: %.az8 $(AZ8)
	$(AZ8) -c -o $@ $<

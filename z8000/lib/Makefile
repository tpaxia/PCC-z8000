# Target runtime objects. Link these with compiler output and crt0.b.
CZ8 = ../cz8/cz8
AZ8 = ../az8/az8

.PHONY: all
all: arith.b float.b softfp.b exit.b csv.b

$(CZ8):
	$(MAKE) -C ../cz8

$(AZ8):
	$(MAKE) -C ../az8

float.az8: gen_float.py
	python3 gen_float.py

softfp.az8: softfp.c $(CZ8)
	$(CZ8) < $< > $@

%.b: %.az8 $(AZ8)
	$(AZ8) -o $@ $<

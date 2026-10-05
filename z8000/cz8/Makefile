XCFLAGS=-O -w -Wno-implicit-int -Wno-implicit-function-declaration -Wno-return-mismatch -Wno-return-type -Wno-int-conversion

cz8: cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o local.o reader.o \
	local2.o order.o match.o allo.o comm1.o table.o
	cc $(XCFLAGS) cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o \
	local.o  reader.o local2.o order.o match.o allo.o comm1.o table.o \
	-o cz8

trees.o: manifest macdefs mfile1 trees.c
	cc -c $(XCFLAGS) trees.c

optim.o: manifest macdefs mfile1 optim.c
	cc -c $(XCFLAGS) optim.c

pftn.o: manifest macdefs mfile1 pftn.c
	cc -c $(XCFLAGS) pftn.c

code.o: manifest macdefs mfile1 code.c
	cc -c $(XCFLAGS) code.c

local.o: manifest macdefs mfile1 local.c
	cc -c $(XCFLAGS) local.c

scan.o: manifest macdefs mfile1 scan.c
	cc -c $(XCFLAGS) scan.c

xdefs.o: manifest mfile1 macdefs xdefs.c
	cc -c $(XCFLAGS) xdefs.c

cgram.o: manifest mfile1 macdefs cgram.c
	cc -c $(XCFLAGS) cgram.c

# cgram.c is generated from cgram.y by the Seventh Edition yacc in ../yacc
# and is never edited by hand. See regen_cgram.py.
cgram.c: cgram.y regen_cgram.py ../yacc/yaccpar ../yacc/y1.c ../yacc/y2.c ../yacc/y3.c ../yacc/y4.c ../yacc/dextern ../yacc/files
	python3 regen_cgram.py

comm1.o: manifest mfile1 common macdefs comm1.c
	cc -c $(XCFLAGS) comm1.c

table.o: manifest mfile2 mac2defs macdefs table.c
	cc -c $(XCFLAGS) table.c

reader.o: manifest mfile2 mac2defs macdefs reader.c
	cc -c $(XCFLAGS) reader.c

local2.o: manifest mfile2 mac2defs macdefs local2.c
	cc -c $(XCFLAGS) local2.c

order.o: manifest mfile2 mac2defs macdefs order.c
	cc -c $(XCFLAGS) order.c

match.o: manifest mfile2 mac2defs macdefs match.c
	cc -c $(XCFLAGS) match.c

allo.o: manifest mfile2 mac2defs macdefs allo.c
	cc -c $(XCFLAGS) allo.c

clean:
	rm -f *.o cz8

/*
 * ioz8.c -- byte order routines for Z8000 object files.
 * Z8000 is big-endian, same as 68000.
 * On a little-endian host, we need to swap bytes for the b.out file.
 */
#include <stdio.h>


get68(file, p, c)
register FILE	*file;
register char	*p;
register c;
{
	if(c == 2) {
		*(short *)p = getc(file);
		*(short *)p = *(short *)p << 8 | getc(file);
	}
	else {
		*(long *)p = getc(file);
		*(long *)p = *(long *)p << 8 | getc(file);
		*(long *)p = *(long *)p << 8 | getc(file);
		*(long *)p = *(long *)p << 8 | getc(file);
	}
}


put68(file, p, c)
register FILE	*file;
register char	*p;
{
	if(c == 2) {
		putc(*(short *)p >> 8, file);
		putc(*(short *)p, file);
	}
	else {
		putc(*(long *)p >> 24, file);
		putc(*(long *)p >> 16, file);
		putc(*(long *)p >> 8, file);
		putc(*(long *)p, file);
	}
}

/* Narrow a numeric value, not the first two bytes of its host storage. */
put16(file, value)
FILE *file;
long value;
{
	putc((unsigned)(value >> 8), file);
	putc((unsigned)value, file);
}

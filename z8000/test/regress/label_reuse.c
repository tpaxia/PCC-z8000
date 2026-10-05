/* Labels have their own name space. A label may reuse the name of a variable
 * that an earlier function declared in an inner block, and of a variable that
 * is still in scope. */
int out1;
inner(a) { { int out; out = a; a = out + 1; } { { int done; done = a; a = done; } } return a; }
jump(a)
{
	if (a) goto out;
	a = 5;
	if (a == 5) goto done;
	a = 9;
out:
	a += 2;
done:
	return a;
}
shared(a)
{
	int again;
	again = 0;
again:
	if (++again < a) goto again;
	out1 = again;
	return again;
}
main()
{
	if (inner(1) != 2) return 1;
	if (jump(0) != 5) return 2;
	if (jump(1) != 3) return 3;
	if (shared(4) != 4 || out1 != 4) return 4;
	return 0;
}

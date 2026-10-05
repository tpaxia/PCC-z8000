/* Multiply, divide and remainder with one unsigned and one int operand are
 * unsigned operations (K&R 6.6), whichever side the unsigned operand is on,
 * for variables as well as constants. */
unsigned uv = 60000;
int iv = 7;
main()
{
	unsigned u;
	int i;
	u = 60000; i = 7;
	if (uv / iv != 8571 || uv % iv != 3) return 1;
	if (u / i != 8571 || u % i != 3 || u / 7 != 8571 || u % 7 != 3) return 2;
	i = 30000; u = 7;
	if (i / u != 4285 || i % u != 5) return 3;
	u = 3; i = 20000;
	if (u * i != 60000 || i * u != 60000) return 4;
	u = 60000; i = 7;
	u /= i; if (u != 8571) return 5;
	u = 60000; u %= i; if (u != 3) return 6;
	u = 9000; u *= i; if (u != 63000) return 7;
	i = 30000; u = 7;
	i /= u; if (i != 4285) return 8;
	i = 30000; i %= u; if (i != 5) return 9;
	return 0;
}

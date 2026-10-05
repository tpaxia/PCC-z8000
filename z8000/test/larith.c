/* larith.c -- test 32-bit arithmetic runtime library
 *
 * Tests: lmul, ulmul, ldiv, lrem, uldiv, ulrem
 *        plus assignment variants (almul, aldiv, etc.)
 *
 * Returns 0 on success, nonzero test number on failure.
 *
 * Note: avoids comparing long values directly against 0L
 * (compiler bug with OPLOG REG/ICON(0) for LONG type).
 * Uses variable-to-variable comparison instead.
 */

long zero;	/* global zero for comparisons */

main()
{
	long a, b, c;
	long *p;
	unsigned long ua, ub, uc;

	/* === Signed multiply (lmul) === */

	/* basic multiply */
	a = 1000L;
	b = 1000L;
	c = a * b;
	if (c != 1000000L)
		return 1;

	/* negative * positive */
	a = -100L;
	b = 200L;
	c = a * b;
	if (c != -20000L)
		return 2;

	/* large multiply -- exercises unsigned correction in al*bl */
	a = 50000L;
	b = 40000L;
	c = a * b;
	if (c != 2000000000L)
		return 3;

	/* multiply by zero -- compare against global zero variable */
	a = 123456L;
	b = 0L;
	c = a * b;
	if (c != zero)
		return 4;

	/* multiply by one */
	a = 123456L;
	b = 1L;
	c = a * b;
	if (c != 123456L)
		return 5;

	/* negative * negative */
	a = -300L;
	b = -400L;
	c = a * b;
	if (c != 120000L)
		return 6;

	/* === Signed divide (ldiv) === */

	a = 1000000L;
	b = 1000L;
	c = a / b;
	if (c != 1000L)
		return 7;

	/* negative dividend */
	a = -1000000L;
	b = 1000L;
	c = a / b;
	if (c != -1000L)
		return 8;

	/* negative divisor */
	a = 1000000L;
	b = -1000L;
	c = a / b;
	if (c != -1000L)
		return 9;

	/* both negative */
	a = -1000000L;
	b = -1000L;
	c = a / b;
	if (c != 1000L)
		return 10;

	/* divide by 1 */
	a = 12345L;
	b = 1L;
	c = a / b;
	if (c != 12345L)
		return 11;

	/* === Signed modulo (lrem) === */

	/* 1000000 % 300 = 100  (since 1000000 = 3333*300 + 100) */
	a = 1000000L;
	b = 300L;
	c = a % b;
	if (c != 100L)
		return 12;

	/* negative dividend: -7 % 3 = -1 (truncation toward zero) */
	a = -7L;
	b = 3L;
	c = a % b;
	if (c != -1L)
		return 13;

	/* exact division: remainder = 0 */
	a = 1000000L;
	b = 1000L;
	c = a % b;
	if (c != zero)
		return 14;

	/* === Unsigned divide -- fast path (divisor < 32768) === */

	ua = 2000000000L;
	ub = 1000L;
	uc = ua / ub;
	if (uc != 2000000L)
		return 15;

	/* unsigned fast path remainder */
	ua = 1000007L;
	ub = 1000L;
	uc = ua % ub;
	if (uc != 7L)
		return 16;

	/* === Unsigned divide -- slow path (divisor >= 32768) === */

	/* divisor with bit 15 set */
	ua = 1000000L;
	ub = 50000L;
	uc = ua / ub;
	if (uc != 20L)
		return 17;

	ua = 1000000L;
	ub = 50000L;
	uc = ua % ub;
	if (uc != zero)
		return 18;

	/* slow path with nonzero remainder */
	ua = 1000001L;
	ub = 50000L;
	uc = ua / ub;
	if (uc != 20L)
		return 19;

	ua = 1000001L;
	ub = 50000L;
	uc = ua % ub;
	if (uc != 1L)
		return 20;

	/* slow path: divisor with nonzero high word */
	ua = 2000000000L;
	ub = 100000L;
	uc = ua / ub;
	if (uc != 20000L)
		return 21;

	ua = 2000000000L;
	ub = 100000L;
	uc = ua % ub;
	if (uc != zero)
		return 22;

	/* dividend < divisor: quotient = 0, remainder = dividend */
	ua = 100L;
	ub = 200L;
	uc = ua / ub;
	if (uc != zero)
		return 23;

	ua = 100L;
	ub = 200L;
	uc = ua % ub;
	if (uc != 100L)
		return 24;

	/* === Assignment variants === */

	/* assignment multiply (*p *= b calls almul) */
	a = 100L;
	p = &a;
	*p *= 200L;
	if (a != 20000L)
		return 25;

	/* assignment divide */
	a = 1000000L;
	p = &a;
	*p /= 1000L;
	if (a != 1000L)
		return 26;

	/* assignment modulo */
	a = 1000000L;
	p = &a;
	*p %= 300L;
	if (a != 100L)
		return 27;

	return 0;
}

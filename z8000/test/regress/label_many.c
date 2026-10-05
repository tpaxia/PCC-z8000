/* A label name must not hold a symbol table slot after its function ends.
 * Each function below uses label names no other function uses; names that
 * are labels in one function are variables in another. */
int total;
f0(a) { if (a) goto aa0; a++; aa0: ab0: ac0: ad0: ae0: af0: ag0: ah0: return a; }
f1(a) { if (a) goto aa1; a++; aa1: ab1: ac1: ad1: ae1: af1: ag1: ah1: return a; }
f2(a) { if (a) goto aa2; a++; aa2: ab2: ac2: ad2: ae2: af2: ag2: ah2: return a; }
f3(a) { if (a) goto aa3; a++; aa3: ab3: ac3: ad3: ae3: af3: ag3: ah3: return a; }
f4(a) { if (a) goto aa4; a++; aa4: ab4: ac4: ad4: ae4: af4: ag4: ah4: return a; }
f5(a) { if (a) goto aa5; a++; aa5: ab5: ac5: ad5: ae5: af5: ag5: ah5: return a; }
f6(a) { if (a) goto aa6; a++; aa6: ab6: ac6: ad6: ae6: af6: ag6: ah6: return a; }
f7(a) { if (a) goto aa7; a++; aa7: ab7: ac7: ad7: ae7: af7: ag7: ah7: return a; }
later(aa0, ab1) { int ac2; ac2 = aa0 + ab1; return ac2; }
int ad3 = 5;
shadow(a)
{
	int done;
	done = a;
	if (done) goto done;
	done = 9;
done:
	return done + ad3;
}
main()
{
	total = f0(0) + f1(0) + f2(0) + f3(0) + f4(1) + f5(1) + f6(1) + f7(1);
	if (total != 8) return 1;
	if (later(2, 3) != 5) return 2;
	if (shadow(1) != 6 || shadow(0) != 14) return 3;
	return 0;
}

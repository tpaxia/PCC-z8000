main()
{
	int i, j, sum;
	sum = 0;
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			if (j == 1) continue;
			if (j == 3) break;
			sum += i + j;
		}
	}
	if (sum != 20) return 1;
	i = 0;
	do { ++i; if (i < 3) continue; sum += i; } while (i < 4);
	if (sum != 27) return 2;
	goto done;
	return 3;
done:
	return 0;
}

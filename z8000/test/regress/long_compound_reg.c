long total;
update(n)
register int n;
{
	long local;
	total += n;
	local = total;
	local -= n;
	if (local != 65530L) return 1;
	return 0;
}
main()
{
	total = 65530L;
	if (update(17) || total != 65547L) return 1;
	total = 65530L;
	if (update(-23) || total != 65507L) return 2;
	return 0;
}

/* KNR: 2.4.3.plain 2.4.3.escapes 2.4.3.octal-escape 2.4.3.is-int */
main()
{
	if ('a' != 97 || 'A' != 65 || '0' != 48 || ' ' != 32) return 1;
	if ('\n' != 10 || '\t' != 9 || '\b' != 8 || '\r' != 13 || '\f' != 12) return 2;
	if ('\\' != 92 || '\'' != 39) return 3;
	if ('\0' != 0 || '\7' != 7 || '\101' != 65 || '\12' != 10) return 4;
	if (sizeof('a') != sizeof(int)) return 5;
	if ('"' != 34) return 6;
	return 0;
}

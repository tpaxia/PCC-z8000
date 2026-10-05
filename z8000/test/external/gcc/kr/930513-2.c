sub3 (i)
     int *i;
{
}

eq (a, b)
{
  static int i = 0;
  if (a != i)
    abort ();
  i++;
}

main ()
{
  int i;

  for (i = 0; i < 4; i++)
    {
      int j = i;
      int k;
      sub3 (&j);
      k = j;
      eq (k, k);
    }
  exit (0);
}

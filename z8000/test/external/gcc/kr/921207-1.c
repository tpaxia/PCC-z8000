f()
{
  unsigned b = 0;

  if (b > ~(unsigned)0)
    b = ~(unsigned)0;

  return b;
}
main()
{
  if (f()!=0)
    abort();
  exit (0);
}

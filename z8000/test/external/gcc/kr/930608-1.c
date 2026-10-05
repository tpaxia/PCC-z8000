double f(a) double a; {}
double (*a[])() = {&f};

main ()
{
  double (*p) ();
  p = &f;
  if (p != a[0])
    abort ();
  exit (0);
}

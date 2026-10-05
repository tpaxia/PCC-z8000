double
f(x) double x;
{
  return x*x;
}

double
Int(f,a) double (*f)(),a;
{
  return (*f)(a);
}

main()
{
  if (Int(&f,2.0) != 4.0)
    abort();
  exit (0);
}

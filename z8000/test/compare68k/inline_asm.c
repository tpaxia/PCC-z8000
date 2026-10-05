int value;
main()
{
 asm("ld r0,#123");
 asm("ld value,r0");
 if(value!=123) return 1;
 return 0;
}

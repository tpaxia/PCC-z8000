int value;
main()
{
 asm("ld r0,#123");
 asm("ld _value,r0");	/* C names carry a leading underscore */
 if(value!=123) return 1;
 return 0;
}

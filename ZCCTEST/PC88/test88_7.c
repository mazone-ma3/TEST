/* z88dk PC-88ƒTƒ“ƒvƒ‹ ALU */

#include <stdio.h>
#include <stdlib.h>

void DI(void){
#asm
	DI
#endasm
}

void EI(void){
#asm
	EI
#endasm
}

unsigned long *vram;

void set_grp(void)
{
	for(vram = (unsigned long *)0xc000; vram < (unsigned long *)0xc000 + 640 /8 * 200; *vram = 0xffffffff, ++vram);
}

void main(void)
{
	unsigned char data=0;

	DI();
	data=inp(0x32);
	data |= 0x40;
	outp(0x32,data); /* USE ALU */

	outp(0x35,0x80); /* Access GVRAM */
/*	outp(0x34,0x07);*/
	outp(0x34,0x05); /* SET */
	set_grp();

	outp(0x34,0x00); /* RESET */
	set_grp();

	outp(0x35,0); /* Access MAINRAM */

	data=inp(0x32);
	data &= 0xbf;
	outp(0x32,data); /* NOUSE ALU */

	EI();
}

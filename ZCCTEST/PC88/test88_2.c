/* z88dk PC-88ƒTƒ“ƒvƒ‹ ALU */
/* zcc +pc88 -DNODELAY -create-app test88_2.c */

#include <stdio.h>
#include <stdlib.h>

unsigned char *vram;

/*	inp();*/

void set_grp(void)
{
	int i;
	for(i = 0; i < 8; i++){
		vram = (unsigned char *)(0xc000 + (640 / 8) * i);
		*vram = 0xff;
		++vram;
		*vram = 0xff;
		++vram;
		*vram = 0xff;
	}
}

void main(void)
{
	unsigned char data=0;
#asm
	DI
#endasm
	data=inp(0x32);
	data |= 0x40;
	outp(0x32,data);

	outp(0x35,0x80);
	outp(0x34,0x07);
	set_grp();
	outp(0x35,0);

	data=inp(0x32);
	data &= 0xbf;
	outp(0x32,data);
#asm
	EI
#endasm
}

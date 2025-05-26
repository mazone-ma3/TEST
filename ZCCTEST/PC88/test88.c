/* z88dk PC-88ƒTƒ“ƒvƒ‹ */
/* zcc +pc88 -DNODELAY -create-app test88.c */

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

	outp(0x5c,data);
	set_grp();
	outp(0x5d,data);
	set_grp();
	outp(0x5e,data);
	set_grp();
	outp(0x5f,data);

#asm
	EI
#endasm
}

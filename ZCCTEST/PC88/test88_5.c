/* z88dk PC-88�T���v�� ALU */
/* zcc +pc88 -DNODELAY -create-app test88_5.c */

#include <stdio.h>
#include <stdlib.h>

unsigned short *vram;

/*	inp();*/

void set_grp(void)
{
	register unsigned char i, j;
	vram = (unsigned short *)0xc000;
	for(i = 0; i < 200; ++i){
/*		vram = (unsigned char *)(0xc000 + (640 / 8) * i);*/
		for(j = 0; j < (640 / 8) / 2; ++j){
			*vram = 0xffff;
			++vram;
		}
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
/*	outp(0x34,0x07);*/
	outp(0x34,0x05);
	set_grp();
	outp(0x35,0);

	data=inp(0x32);
	data &= 0xbf;
	outp(0x32,data);
#asm
	EI
#endasm
}

/* z88dk PC-88ƒTƒ“ƒvƒ‹ ALU */
/* zcc +pc88 -DNODELAY -create-app test88_6.c */

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

unsigned short *vram;

/*	inp();*/

void set_grp(void)
{
	register unsigned char i, j, k;
	vram = (unsigned short *)0xc000;
	for(j = 0;j < 8; ++j){
		for(i = 0; i < 200 / 8; ++i){
			vram = (unsigned short *)(0xc000 + (640 / 8) * (i * 8 + j));
			for(k = 0; k < (640 / 8) / 2; ++k){
				*vram = 0xffff;
				++vram;
			}
		}
	}
}

void reset_grp(void)
{
	register unsigned char i, j, k;
	vram = (unsigned short *)0xc000;
	for(j = 0;j < 8; ++j){
		for(i = 0; i < 200 / 8; ++i){
			vram = (unsigned short *)(0xc000 + (640 / 8) * (i * 8 + j));
			for(k = 0; k < (640 / 8) / 2; ++k){
				*vram = 0x0;
				++vram;
			}
		}
	}
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

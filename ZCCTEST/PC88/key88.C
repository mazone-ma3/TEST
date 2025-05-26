/* z88dk PC-88ƒTƒ“ƒvƒ‹ MACHINE & KEY */

#include <stdio.h>
#include <stdlib.h>

unsigned char *mode_syswork = (unsigned char)0xe6c2;

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

void Set_RAM_MODE(void){
#asm
	DI
#endasm
	outp(0x31, *mode_syswork | 0x02);
}

void Set_ROM_MODE(void){
	outp(0x31, *mode_syswork);
#asm
	EI
#endasm
}

unsigned char romport;

void set_mainROM(void)
{
	romport = inp(0x71);
	outp(0x71, 0xff);
}

void reset_mainROM(void)
{
	outp(0x71, romport);
}

void main(void)
{
	unsigned char *mem;
	unsigned char machine, mode, clock;
	unsigned char k0, k1, k8, ka, st, pd, k5, k9;

	mem = (unsigned char *)0x79d7;
	set_mainROM();
	machine = *mem;
	reset_mainROM();

	mode = inp(0x31) & 0xc0;
	clock = inp(0x6e) & 0x80;

	printf("machine=%c mode=%X clock=%X\n", machine, mode, clock);

	while(((k9 = inp(0x09)) & 0x80)){ /* ESC */
		k0 = inp(0x00);
		k1 = inp(0x01);
		k8 = inp(0x08);
		ka = inp(0x0a);
		outp(0x44, 0x0e);
		st = inp(0x45);
		outp(0x44, 0x0f);
		pd = inp(0x45);
		k5 = inp(0x05);
		if(!(k1 & 0x01) || !(k8 & 0x02) || !(st & 0x01)) /* 8 */
			printf("up");
		if(!(k0 & 0x40) || !(k8 & 0x04) || !(st & 0x08)) /* 6 */
			printf("right");
		if(!(k0 & 0x04) || !(ka & 0x02) || !(st & 0x02)) /* 2 */
			printf("down");
		if(!(k0 & 0x10) || !(ka & 0x04) || !(st & 0x04)) /* 4 */
			printf("left");
		if(!(k5 & 0x04) || !(k9 & 0x40) || !(pd & 0x01)) /* Z,SPACE */
			printf("A");
		if(!(k5 & 0x01) || !(pd & 0x02)) /* X */
			printf("B");
	}
}


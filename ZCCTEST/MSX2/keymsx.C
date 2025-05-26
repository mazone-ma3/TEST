/* z88dk MSXサンプル MACHINE & KEY */

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

/* mainromの指定番地の値を得る */
int read_mainrom(int adr)
{
#asm
	ld	 hl, 2
	add	hl, sp
	ld	e, (hl)
	inc	hl
	ld	d, (hl)	; de=adr
	ld	h,d
	ld	l,e	; hl=adr

	ld	a,($fcc1)	; exptbl
	call	$000c	; RDSLT

	ld	l,a
	ld	h,0
#endasm
}


int get_key(int matrix)
{
#asm
	ld	a,($fcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,$0141	; SNSMAT(MAINROM)

	ld	 hl, 2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	$001c	; CALSLT

	ld	l,a
	ld	h,0
#endasm
}

int get_stick(int trigno)
{
#asm
	ld	a,($fcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,$00d5	; GTSTCK(MAINROM)

	ld	 hl, 2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	$001c	; CALSLT
	ld	l,a
	ld	h,0
#endasm
}

int get_pad(int trigno)
{
#asm
	ld	a,($fcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,$00d8	; GTTRIG(MAINROM)

	ld	 hl, 2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	$001c	; CALSLT
	ld	l,a
	ld	h,0
#endasm
}

void main(void)
{
	unsigned char machine, mode1, mode2;
	unsigned char st0, st1, pd0, pd1, pd2, k3, k5, k7, k9, k10;

	machine = read_mainrom(0x2d);
	mode1 = read_mainrom(0x2b);
	mode2 = read_mainrom(0x2c);

	printf("machine=%X mode1=%X mode2=%X\n", machine, mode1, mode2);

	while(((k7 = get_key(7)) & 0x04)){ /* ESC */
		st0 = get_stick(0);
		st1 = get_stick(1);

		pd0 = get_pad(0);
		pd1 = get_pad(1);
		pd2 = get_pad(3);

		k3 = get_key(3);
		k9 = get_key(9);
		k10 = get_key(10);
		k5 = get_key(5);

		if((st0 >= 1 && st0 <=2) || (st0 == 8) || (st1 >= 1 && st1 <=2) || (st1 ==8) || !(k10 & 0x08)) /* 8 */
			printf("up");
		if((st0 >= 2 && st0 <=4) || (st1 >= 2 && st1 <=4) || !(k10 & 0x02)) /* 6 */
			printf("right");
		if((st0 >= 4 && st0 <=6) || (st1 >= 4 && st1 <=6) || !(k9 & 0x20)) /* 2 */
			printf("down");
		if((st0 >= 6 && st0 <=8) || (st1 >= 6 && st1 <=8) || !(k9 & 0x80)) /* 4 */
			printf("left");
		if((pd0) || (pd1) || !(k5 & 0x20)) /* X,SPACE */
			printf("A");
		if((pd2) || !(k3 & 0x01)) /* C */
			printf("B");
	}
}


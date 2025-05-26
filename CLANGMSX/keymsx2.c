/* z88dk MSXサンプル MACHINE & KEY CLANG */

#include <stdio.h>
#include <stdlib.h>

unsigned char *mode_syswork = (unsigned char *)0xe6c2;

inline void DI(void)
{
asm volatile(
	"di\n"
);
}

inline void EI(void)
{
asm volatile(
	"ei\n"
);
}

unsigned short adr;
unsigned char no;

/* mainromの指定番地の値を得る */
static unsigned char read_mainrom(int adr)
{
	register unsigned char ra asm("a");
	register short rhl asm("hl");
	rhl = adr;

asm volatile(
	"ld	a,(0xfcc1)	; exptbl\n"
	"call	0x000c	; RDSLT\n"

	:"=r"(ra)	/* 値が返るレジスタ変数 */
	:"r"(rhl)	/* 引数として使われるレジスタ変数 */
	:"ix", "iy", "de", "bc"		/* 破壊されるレジスタ */
);
	return ra;
}


static unsigned char get_key(unsigned char matrix)
{
	register unsigned char ra asm("a");
	register unsigned char rd asm("d") = matrix;

asm volatile(
	"push	ix\n"

	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x0141	; SNSMAT(MAINROM)\n"

	"ld	a,%1\n"

	"call	0x001c	; CALSLT\n"

	"pop	ix\n"

	:"=r"(ra)	/* 値が返るレジスタ変数 */
	:"r"(rd)	/* 引数として使われる変数 */
	: "bc", "iy"	/* 破壊されるレジスタ */
);
	return ra;
}

static unsigned char get_stick(unsigned char trigno)
{
	register unsigned char ra asm("a");
	register unsigned char rd asm("d") = trigno;

asm volatile(
	"push	de\n"
	"push	ix\n"

	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x00d5	; GTSTCK(MAINROM)\n"
	"ld	a,%1\n"

	"call	0x001c	; CALSLT\n"

	"pop	ix\n"
	"pop	de\n"

	:"=r"(ra)	/* 値が返るレジスタ変数 */
	:"r"(rd)	/* 引数として使われる変数 */
	: "bc", "iy","hl"	/* 破壊されるレジスタ */
);

	return ra;
}

static unsigned char get_pad(unsigned char trigno)
{
	register unsigned char ra asm("a");
	register unsigned char rd asm("d") = trigno;

asm volatile(
	"push	ix\n"

	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x00d8	; GTTRIG(MAINROM)\n"

	"ld	a,%1\n"
	"call	0x001c	; CALSLT\n"

	"pop	ix\n"

	:"=r"(ra)	/* 値が返るレジスタ変数 */
	:"r"(rd)	/* 引数として使われる変数 */
	: "bc", "iy"	/* 破壊されるレジスタ */
);

	return ra;
}

int main(void)
{
	unsigned char machine, mode1, mode2;
	unsigned char st0, st1, pd0, pd1, pd2, k3, k5, k7, k9, k10;

	DI();
	EI();

	machine = read_mainrom(0x2d);
	mode1 = read_mainrom(0x2b);
	mode2 = read_mainrom(0x2c);

	printf("machine=%X mode1=%X mode2=%X\n", machine, mode1, mode2);

	while(1){
		k7 = get_key(7);
		if(!(k7 & 0x04))
			break;

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

asm volatile(
	"push	ix\n"
	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x0156	; KILBUF(MAINROM)\n"
	"call	0x001c	; CALSLT\n"
	"pop	ix\n"

	:	/* 値が返るレジスタ変数 */
	:	/* 引数として使われるレジスタ変数 */
	:"a", "iy", "ix", "hl", "de", "bc"	/* 破壊されるレジスタ */
);
	return 0;
}


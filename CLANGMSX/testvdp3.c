/* z88dk MSX2サンプル VDP CLANG */

#include <stdio.h>
#include <stdlib.h>

#define SPR_ATR_ADR 0x7600
#define SPR_PAT_ADR 0x7800
#define SPR_COL_ADR (SPR_ATR_ADR-512)

enum {
	VDP_READDATA = 0,
	VDP_READSTATUS = 1
};

enum {
	VDP_WRITEDATA = 0,
	VDP_WRITECONTROL = 1,
	VDP_WRITEPAL = 2,
	VDP_WRITEINDEX = 3
};

unsigned char *oldscr = (unsigned char *)0xfcb0;
unsigned char vdp_readport[4], vdp_writeport[4];

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

/* screenのBIOS切り替え */
static void set_screenmode(unsigned char mode)
{
	register unsigned char rd asm("d") = mode;

asm volatile(
	"push	de\n"
	"push	ix\n"

	"ld	a,(0xfcc1)	; exptbl\n"
	"ld	b,a\n"
	"ld	c,0\n"
	"push	bc\n"
	"pop	iy\n"
	"ld ix,0x005f	; CHGMOD(MAINROM)\n"
	"ld	a,%0\n"

	"call	0x001c	; CALSLT\n"

	"pop	ix\n"
	"pop	de\n"

	:	/* 値が返るレジスタ変数 */
	:"r"(rd)	/* 引数として使われる変数 */
	: "bc", "iy","hl"	/* 破壊されるレジスタ */
);
}

void write_vdp(unsigned char regno, unsigned char data)
{
	outp(vdp_writeport[VDP_WRITECONTROL], data);
	outp(vdp_writeport[VDP_WRITECONTROL], 0x80 | regno);
}

void write_vram_adr(unsigned char highadr, int lowadr)
{
	write_vdp(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
	outp(vdp_writeport[VDP_WRITECONTROL], (lowadr & 0xff));
	outp(vdp_writeport[VDP_WRITECONTROL], 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data)
{
	outp(vdp_writeport[VDP_WRITEDATA], data);
}

void set_screen5(void)
{
	DI();
	write_vdp(0, 0x06);
	write_vdp(1, 0x60);
	write_vdp(8, 0x08);
	write_vdp(9, 0x88);
	EI();
}

void set_displaypage(int page)
{
	DI();
	write_vdp(2, (page << 5) & 0x60 | 0x1f);
	EI();
}

void set_screen1(void)
{
	DI();
	write_vdp(0, 0x00);
	write_vdp(1, 0x60);
	write_vdp(8, 0x08);
	write_vdp(9, 0x08);
	EI();
}

void set_bgcolor(unsigned char color)
{
	DI();
	write_vdp(7, color & 0x0f);
	EI();
}

void set_spr_atr_adr(unsigned char highadr, int lowadr)
{
	DI();
	write_vdp(5, (lowadr >> (2 + 5)) & 0xf8 | 0x07);
	write_vdp(11, ((highadr << 1) & 0x02) | ((lowadr >> 15) & 0x01));
	EI();
}

void set_spr_pat_adr(unsigned char highadr, int lowadr)
{
	DI();
	write_vdp(6, ((highadr << 5) & 0x20) | ((lowadr >> 11) & 0x1f));
	EI();
}

void spr_on(void)
{
	DI();
	write_vdp(8, 0x00);
	EI();
}

void spr_off(void)
{
	DI();
	write_vdp(8, 0x02);
	EI();
}

int main(void)
{
	int i;
	unsigned char vdp_readadr;
	unsigned char vdp_writeadr;

	vdp_readadr = read_mainrom(6);
	vdp_writeadr = read_mainrom(7);

	for(i = 0; i < 4; i++){
		vdp_readport[i] = vdp_readadr + i;
		vdp_writeport[i] = vdp_writeadr + i;
	}

//	set_screen5();
	set_screenmode(5);
	set_displaypage(0);

	set_bgcolor(1);
	set_spr_atr_adr(0, SPR_ATR_ADR); /* color table : atr-512 (0x7400) */
	set_spr_pat_adr(0, SPR_PAT_ADR);

	DI();
	write_vram_adr(0, SPR_ATR_ADR);
	for(i = 0; i < 32; ++i){
		write_vram_data(255);		/* Y */
		write_vram_data(0);		/* X */
		write_vram_data(0);		/* No */
		write_vram_data(0);		/* NoUse */
	}
	EI();

	DI();
	write_vram_adr(0, SPR_PAT_ADR);
	for(i = 0; i < (8 * 4); ++i){
		write_vram_data(0xff);
	}
	EI();

	DI();
	write_vram_adr(0, SPR_COL_ADR);
	for(i = 0; i < 16; ++i){
		write_vram_data(15 - i);
	}
	EI();

	DI();
	write_vram_adr(0, 0);
	for(i = 0; i < (256 / 2) * 212; i++){
/*		write_vram_adr(0, i); */
		write_vram_data(0xff);
	}
	EI();

	set_displaypage(1);
	DI();
	write_vram_adr(0, 0x8000);
	for(i = 0; i < (256 / 2) * 212; i++){
		write_vram_data(0xee);
	}
	EI();

	set_displaypage(2);
	DI();
	write_vram_adr(1, 0);
	for(i = 0; i < (256 / 2) * 212; i++){
		write_vram_data(0xdd);
	}
	EI();

	getchar();
	/*set_screen1();*/
	set_screenmode(*oldscr);

	return 0;
}

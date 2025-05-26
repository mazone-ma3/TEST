/* z88dk MSX2サンプル VDP */

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

unsigned char vdp_readport[4], vdp_writeport[4];

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

/* screenのBIOS切り替え */
void set_screenmode(int mode)
{
#asm
	ld	a,($fcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,$005f	; CHGMOD(MAINROM)

	ld	 hl, 2
	add	hl, sp
	ld	a, (hl)	; a = mode

	call	$001c	; CALSLT
#endasm
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

unsigned char read_VDPstatus(int no)
{
	unsigned char data;
	DI();
	write_vdp(15, no);
	data = inp(vdp_readport[VDP_READSTATUS]);
	write_vdp(15, 0);
	EI();
	return data;
}


void boxfill(int dx, int dy, int nx, int ny, unsigned char dix, unsigned char diy, unsigned char data)
{
	write_vdp(17, 36);

	outp(vdp_writeport[VDP_WRITEINDEX], dx & 0xff);
	outp(vdp_writeport[VDP_WRITEINDEX], (dx >> 8) & 0x01);
	outp(vdp_writeport[VDP_WRITEINDEX], dy & 0xff);
	outp(vdp_writeport[VDP_WRITEINDEX], (dy >> 8) & 0x03);
	outp(vdp_writeport[VDP_WRITEINDEX], nx & 0xff);
	outp(vdp_writeport[VDP_WRITEINDEX], (nx >> 8) & 0x01);
	outp(vdp_writeport[VDP_WRITEINDEX], ny & 0xff);
	outp(vdp_writeport[VDP_WRITEINDEX], (ny >> 8) & 0x03);
	outp(vdp_writeport[VDP_WRITEINDEX], data);
	outp(vdp_writeport[VDP_WRITEINDEX], ((diy << 3) & 0x80) | ((diy << 2) & 0x40));
	outp(vdp_writeport[VDP_WRITEINDEX], 0xc0);
}

void main(void)
{
	int i;
	unsigned char vdp_readadr;
	unsigned char vdp_writeadr;

	vdp_readadr = read_mainrom(0x0006);
	vdp_writeadr = read_mainrom(0x0007);

	for(i = 0; i < 4; i++){
		vdp_readport[i] = vdp_readadr + i;
		vdp_writeport[i] = vdp_writeadr + i;
	}

	set_screen5();
	set_displaypage(0);
/*	set_screenmode(5);*/

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

	while(read_VDPstatus(2) & 0x01);
	DI();
	boxfill(0, 0, 256, 212, 0, 0, 0xff);
	EI();
	while(read_VDPstatus(2) & 0x01);
	getchar();
	boxfill(0, 0, 256, 212, 0, 0, 0x00);
	EI();
	while(read_VDPstatus(2) & 0x01);
	getchar();

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
	set_screenmode(1);
}

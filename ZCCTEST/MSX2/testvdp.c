/* z88dk MSX2サンプル VDP */
/* zcc +msx -DNODELAY -lm -create-app -subtype=msxdos testvdp.c -o testvdp.com */

#include <stdio.h>
#include <stdlib.h>

unsigned char vdp_readadr;
unsigned char vdp_writeadr;

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
	ld	a, (hl) ; a = mode

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

void main(void)
{
	int i;

	vdp_readadr = read_mainrom(0x0006);
	vdp_writeadr = read_mainrom(0x0007);

	for(i = 0; i < 4; i++){
		vdp_readport[i] = vdp_readadr + i;
		vdp_writeport[i] = vdp_writeadr + i;
	}

	set_screen5();
	set_displaypage(0);
/*	set_screenmode(5);*/

	DI();
	write_vram_adr(0, 0);
	for(i = 0; i < (256 / 2) * 212; i++){
		write_vram_adr(0, i);
		write_vram_data(0xff);
	}
	write_vram_adr(0, 0);
	for(i = 0; i < (256 / 2) * 212; i++){
		write_vram_data(0x11);
	}
	EI();

	getchar();
	/*set_screen1();*/
	set_screenmode(1);
}

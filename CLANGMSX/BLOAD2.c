/* MSX-SC5 Loader for eZ80-CLANG(MSX-DOS) By m@3 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <fcntl.h>

#define ERROR 1
#define NOERROR 0

#define WIDTH 128
#define LINE 212

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

#define VDP_readport(no) (VDP_readadr + no)
#define VDP_writeport(no) (VDP_writeadr + no)

unsigned char VDP_readadr;
unsigned char VDP_writeadr;

#define MAXCOLOR 16

short  i, count, n = 0;
short m = 0;

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

void write_VDP(unsigned char regno, unsigned char data)
{
	outp(VDP_writeport(VDP_WRITECONTROL), data);
	outp(VDP_writeport(VDP_WRITECONTROL), 0x80 | regno);
}

void write_vram_adr(unsigned char highadr, short lowadr)
{
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data)
{
	outp(VDP_writeport(VDP_WRITEDATA), data);
}

void set_displaypage(unsigned char page)
{
	asm("DI\n");
	write_VDP(2, (page << 5) & 0x60 | 0x1f);
	asm("EI\n");
}

/*終了処理*/
void end(void)
{
asm(
	"xor	a\n"
	"ld	c,0\n"
	"call	0x0005\n"

	:	/* 値が返るレジスタ変数 */
	:	/* 引数として使われるレジスタ変数 */
	:"a", "iy", "ix", "hl", "bc"//, "de"		/* 破壊されるレジスタ */
);
}

unsigned short vram_start_adr, vram_end_adr;
unsigned char page = 0;
unsigned char mode = 0;

char *filename;
short inhandle;


unsigned char *oldscr = (unsigned char *)0xfcb0;

unsigned char pattern[32767];

int	main(int argc,char **argv){

	if (argc < 2){
		printf("MSX .SC5 file Loader for MSX2.\n");
		return ERROR;
	}

	if (argc >= 3){
		page = atoi(argv[2]);
		if(page > 3)
			page = 0;
	}

	if(argc < 4)
		mode = 1;

	filename = argv[1];

	inhandle = open( filename, O_RDONLY | O_BINARY , 0);

	if (inhandle == -1) {
		printf("Can\'t open file %s.\n", filename);

		return ERROR;
	}

	read( inhandle, pattern, 32767);
	/* MSX先頭ヘッダ */
	if(pattern[0] != 0xfe){
		printf("Not BSAVE,S file %s.\n", filename);
		close(inhandle);
		end();
		return ERROR;
	}
	/* MSXヘッダ 開始アドレス */
	vram_start_adr = pattern[1] + pattern[2] * 256;

	/* MSXヘッダ 終了アドレス */
	vram_end_adr = pattern[3] + pattern[4] * 256;

	VDP_readadr = read_mainrom(6);
	VDP_writeadr = read_mainrom(7);

	set_screenmode(5);

	switch(page){
		case 1:
			write_vram_adr(0x00, vram_start_adr + 0x8000);
			break;
		case 2:
			write_vram_adr(0x01, vram_start_adr);
			break;
		case 3:
			write_vram_adr(0x01, vram_start_adr + 0x8000);
			break;
		default:
			write_vram_adr(0x00, vram_start_adr);
			break;
	}
	if(page < 4)
		if(mode)
			set_displaypage(page);

	n = vram_start_adr;

	m = 7;
	for(count = 0; count < (WIDTH * LINE); ++count){
		write_vram_data(pattern[m]);
		if(n == vram_end_adr){
			break;
		}
		++m;
		++n;
	}
	close(inhandle);

	if(mode)
		getch();

	set_screenmode(*oldscr);
	end();

	return NOERROR;
}

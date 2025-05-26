/* MSX-SC5->PC-AT CONV.(Analog 16 colors) for GCC-ia16 By m@3 */
/* 全体版 */

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define ON 1
#define OFF 0
#define PAT 1

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))

/* BITセット */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

#define WIDTH 32
#define LINE 212
#define RAM_ADR 0x2000


unsigned char __far *vram_adr = MK_FP(0xa000,0);

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};

void cursor_switch(short);
void screen_switch(short);


int conv(char *loadfil)
{
	long i, j,count, count2;
	int k=0, l=0, m=0;
	unsigned char read_pattern[WIDTH * LINE * 2+ 2];
	unsigned char pattern[WIDTH * LINE + 2];
	unsigned char pcatcolor[4];
	unsigned char msxcolor[8];
	unsigned char color;
	unsigned short header;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */


	for(count = 0; count < 2; ++count){
		i = fread(read_pattern, 1, WIDTH * LINE * 2, stream[0]);	/* 4dot分 */
		m = 0;
//		if(i < 1)
//			break;
		for(count2 = 0; count2 < WIDTH * LINE / 2; ++count2){
	

			/* 色分解と拡大 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[2] = read_pattern[m] & 0x0f;
			msxcolor[3] = read_pattern[m] & 0x0f;
			msxcolor[4] = (read_pattern[++m] >>4) & 0x0f;
			msxcolor[5] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[6] = read_pattern[m] & 0x0f;
			msxcolor[7] = read_pattern[m++] & 0x0f;
			for(i = 0; i < 4; ++i){
				pcatcolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pcatcolor[i]);
					}else{
						BITCLR(7-j, pcatcolor[i]);
					}
				}
			}

//			i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
//			if(i < 1)
//				break;

			/* 色分解と拡大 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[2] = read_pattern[m] & 0x0f;
			msxcolor[3] = read_pattern[m] & 0x0f;
			msxcolor[4] = (read_pattern[++m] >>4) & 0x0f;
			msxcolor[5] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[6] = read_pattern[m] & 0x0f;
			msxcolor[7] = read_pattern[m++] & 0x0f;

			for(i = 0; i < 4; ++i){
				pattern[i] = pcatcolor[i];
			}

			for(i = 0; i < 4; ++i){
				pcatcolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pcatcolor[i]);
					}else{
						BITCLR(7-j, pcatcolor[i]);
					}
				}
			}

			for(i = 0; i < 4; ++i){
				pattern[4 + i] = pcatcolor[i];
			}

			outportb(0x3ce, 0x05);	// write mode
			outportb(0x3cf, 0x00);	// normal
			outportb(0x3ce, 0x08);	// bit mask
			outportb(0x3cf, 0xff);	// none-mask

			outportb(0x3c4, 0x02); 	// map mask
			outportb(0x3c5, 0x01);	// plane
			*(vram_adr + 0 + k + l) = pattern[0];
			*(vram_adr + 1 + k + l) = pattern[4];
			*(vram_adr + 80 + k + l) = pattern[0];
			*(vram_adr + 81 + k + l) = pattern[4];

			outportb(0x3c4, 0x02);
			outportb(0x3c5, 0x02);
			*(vram_adr + 0 + k + l) = pattern[1];
			*(vram_adr + 1 + k + l) = pattern[5];
			*(vram_adr + 80 + k + l) = pattern[1];
			*(vram_adr + 81 + k + l) = pattern[5];

			outportb(0x3c4, 0x02);
			outportb(0x3c5, 0x04);
			*(vram_adr + 0 + k + l) = pattern[2];
			*(vram_adr + 1 + k + l) = pattern[6];
			*(vram_adr + 80 + k + l) = pattern[2];
			*(vram_adr + 81 + k + l) = pattern[6];

			outportb(0x3c4, 0x02);
			outportb(0x3c5, 0x08);
			*(vram_adr + 0 + k + l) = pattern[3];
			*(vram_adr + 1 + k + l) = pattern[7];
			*(vram_adr + 80 + k + l) = pattern[3];
			*(vram_adr + 81 + k + l) = pattern[7];

			k += 2;
			if(k >= 64){
				k = 0;
				l += 80*2;
			}
		}
	}
	fclose(stream[0]);

	return 0;
}

union REGS reg;
union REGS reg_out;

unsigned char old_mode;

void g_init(void)
{
	reg.h.ah = 0x0f;
	int86(0x10, &reg, &reg);
	old_mode = reg.h.al;

	reg.h.ah = 0x00;
	reg.h.al = 0x12; //0x72;	/* VGA mode 640x480/16colors */
	int86(0x10, &reg, &reg);
}

/*終了処理*/
void end()
{
	reg.h.ah = 0x00;
	reg.h.al = old_mode;
	int86(0x10, &reg, &reg);
}

void paint(unsigned char plane, unsigned char bit)
{
	unsigned short i, j;

	for (i = 0; i < (480L); ++i){
		for (j = 0; j < 80; ++j){
			outportb( 0x3c4, 0x02);		/* mask */
			outportb( 0x3c5, plane);		/* plane */
			*(vram_adr + j + i * 80) = bit; /* bit */;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1)
		paint(0x0f, 0);

	if(type & 2)
		printf("\x1b*");
}


/*パレット・セット*/
void pal_set(unsigned char color, unsigned char red, unsigned char blue,
	unsigned char green)
{
/*	reg.h.ah = 0x10;
	reg.h.al = 0x10;
	reg.x.bx = color;
	reg.h.dh = red;
	reg.h.ch = blue;
	reg.h.cl = green;
	int86(0x10, &reg, &reg);
*/
	outportb(0x3c8, color);
	outportb(0x3c9, red);
	outportb(0x3c9, blue);
	outportb(0x3c9, green);
}

void pal_all(unsigned char color[16][3])
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(i, ((color[i][0] + 1)*4-1) * (color[i][0] != 0), ((color[i][1]+1)*4-1) * (color[i][1] != 0), ((color[i][2]+1)*4-1) * (color[i][2] != 0));
}


int	main(int argc,char **argv){

	short i,j;
	unsigned short a, b;

	unsigned char pal[16][3] = {
		{  0,  0,  0},
		{  0,  0,  0},
		{  3, 13,  3},
		{  7, 15,  7},
		{  3,  3, 15},
		{  5,  7, 15},
		{ 11,  3,  3},
		{  5, 13, 15},
		{ 15,  3,  3},
		{ 15,  7,  7},
		{ 13, 13,  3},
		{ 13, 13,  7},
		{  3,  9,  3},
		{ 13,  5, 11},
		{ 11, 11, 11},
		{ 15, 15, 15},
	};

	if (argv[1] == NULL){
		printf("MSX .SC5 file Converter for PC-AT.\n");
		return 1;
	}
	g_init();
	for(i = 0; i < 16; ++i){
		reg.x.ax = 0x1000;
		reg.h.bl = i;
		reg.h.bh = i;

		int86(0x10, &reg, &reg);
	}
	pal_all(pal);

	clear(3);
	conv(argv[1]);

	getch();
	end();

	return 0;
}

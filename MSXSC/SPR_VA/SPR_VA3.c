/* PC-88VA GCC-ia16 スプライト表示実験  By m@3 */
/* キャラを出す */
#define _BORLANDC_SOURCE

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <i86.h>
#include <fcntl.h>
#include <dos.h>
#include <unistd.h>

#define CHR_TOP 32
enum {
	IMG_SIZE_X = 16
};
#define IMG_SET(X, Y)	(CHR_TOP + X + IMG_SIZE_X * Y)

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

#define ON 1
#define OFF 0
#define ERROR -1
#define NOERROR 0

#define MAXCOLOR 16
#define ON 1
#define OFF 0

#define SPR_SIZE(X,Y) (X / 2 * Y)


void g_init(void);
void end(void);
void clear(unsigned short);
void pal_set(unsigned short,unsigned char,unsigned char,unsigned char);
void pal_all(unsigned char[16][3]);


unsigned short x = 0, y = 0, xx, yy, old_x = 0, old_y = 0;
unsigned char dir = 2, dir2 = 0, i, j, k;

unsigned char __far *bvram, // = (unsigned char __far *)MK_FP(0xa000, 0),
	 *rvram, // = (unsigned char __far *)MK_FP(0xb000, 0),
	 *gvram, // = (unsigned char __far *)MK_FP(0xc000, 0),
	 *ivram; // = (unsigned char __far *)MK_FP(0xd000, 0);

unsigned char __far *spr_atr;

unsigned char __far *flame[4]
	 = {MK_FP(0xa000,0)	,MK_FP(0xb000,0),MK_FP(0xc000,0),MK_FP(0xd000,0)};

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};

short set_sprite_pattern(char *loadfil, unsigned short adr, unsigned short width, unsigned short line, unsigned char spr_x_size, unsigned char spr_y_size, unsigned char spr_x_max, unsigned char spr_y_max)
{
	long i, j,count, count2;
	unsigned short k=0, l=0, m;
	unsigned char pattern[100];
	unsigned short header;

	unsigned short spr_x = 0, spr_y = 0, spr_no = 0, spr_no2 = 0;
	unsigned short index = 0;
	unsigned char __far *tvram = MK_FP(0xa000, 0);
	unsigned short spr_size = SPR_SIZE(spr_x_size, spr_y_size);

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return ERROR;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */

	for(spr_no2 = 0; spr_no2 < spr_y_max; ++spr_no2){
		spr_y = spr_no2 * (spr_y_size) * spr_x_max;
		for(count = 0; count < line; ++count){

			for(count2 = 0; count2 < (width / 4); ++count2){

				i = fread(pattern, 1, 2, stream[0]);	/* 8dot分 */
				if(i < 1)
					break;

				index = ((spr_no) * spr_size) + spr_x + (spr_y * (spr_x_size / 2)) ;

				/* 横を2倍する */
				for(m = 0; m < 1; ++m){
					tvram[adr + index] = 
						(((pattern[m * 2 + 0]) >> 4) & 0x0f) |
						(((pattern[m * 2 + 0]) >> 4) & 0x0f) * 16;
					++index;
					tvram[adr + index] = 
						(((pattern[m * 2 + 0]) & 0x0f)) |
						(((pattern[m * 2 + 0]) & 0x0f) * 16);
					++index;
					tvram[adr + index] = 
						(((pattern[m * 2 + 1]) >> 4) & 0x0f) |
						(((pattern[m * 2 + 1]) >> 4) & 0x0f) * 16;
					++index;
					tvram[adr + index] = 
						(((pattern[m * 2 + 1]) & 0x0f)) |
						(((pattern[m * 2 + 1]) & 0x0f) * 16);
					++index;
				}

				spr_x += 4;
				if(spr_x >= (spr_x_size / 2)){
					spr_x = 0;
					++spr_no;
					if(spr_no >= spr_x_max){
						spr_no = 0;
						++spr_y;
					}
				}
			}
			for(count2 = 0; count2 < ((256 - width) / 4); ++count2){

				i = fread(pattern, 1, 2, stream[0]);	/* 8dot分 */
				if(i < 1)
					break;
			}
		}
	}
	fclose(stream[0]);

	return index;
}

void g_init(void)
{
//	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
//	outportw(0x100, 0xb020);	/* none-interless Graphic-on 200dot(200line) */
	outportw(0x100, 0xb062);	/* none-interless Graphic-on 400dot(200line) */
								/* 画面ON notsingle-plane 1画面 */

//	outportw(0x102, 0x0101);	/* graphic0/1 Width640 4dot/pixel */
	outportw(0x102, 0x1111);	/* graphic0/1 Width320 4dot/pixel */

//	outportb(0x153, 0x44);		/* G-VRAM選択 */
	outportb(0x153, 0x41);		/* T-VRAM選択 */

	outportw(0x106, 0xab89);	/* パレット指定画面割当て指定 */
	outportw(0x108, 0x0000);	/* 直接色指定画面割当て設定 */
	outportw(0x110, 0x008f);	/* 4ビットピクセル */
//	outportb(0x500, 0);	/* 独立アクセス */
//	outportb(0x512, 0);	/* ブロック0 */
//	outportb(0x516, 0);	/* 書き込みプレーン選択 */

	outportb(0x10d, 0x01);
	outportb(0x10c, 0x00);	/* カラーパレットモード */
}


/*終了処理*/
void end()
{
	union REGS reg;
	union REGS reg_out;

	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
	outportw(0x102, 0x0101);	/* graphic0 Width640 4dot/pixel */
	outportb(0x153, 0x41);		/* T-VRAM選択 */
	outportw(0x106, 0xab89);	/* パレット指定画面割当て指定 */
	outportb(0x10d, 0x01);
	outportb(0x10c, 0x80);	/* カラーパレットモード */
//	outportb(0x10c, 0x10);	/* カラーパレットモード */

	reg.h.ah = 0x2a;
	int86(0x83, &reg, &reg_out);	/* テキスト初期化 */
}


void paint(unsigned char pattern)
{
	unsigned short i, j;

	for (i = 0; i < (80 * 200L); ++i){
		for(j = 0; j < 4; j++){
			*(flame[j] + i) = pattern;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(unsigned short type)
{
	if(type & 1)
		paint(0x0);

//	if(type & 2)
//		printf("\x1b*");
}

/*パレット・セット*/
void pal_set(unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outportw(0x300 + color * 2, (unsigned short)green * 4096 + red * 64 + blue * 2);
//	outportw(0x320 + color * 2, (unsigned short)green * 4096 + red * 64 + blue * 2);
}

void pal_all(unsigned char color[16][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(i, color[i][0], color[i][1], color[i][2]);
}

#define SPR_PAT_ADR 0xaa00

void set_sprite_locate(unsigned short pat_no, unsigned short chr_adr,  unsigned short x, unsigned short y, unsigned char spr_x_size, unsigned char spr_y_size){
	unsigned char __far *spr = &spr_atr[pat_no * 8];
	unsigned short spr_size = SPR_SIZE(spr_x_size, spr_y_size);
	*(spr++) = y % 256;				/* Y */
	*(spr++) = (spr_y_size / 4 - 1) * 4 | 0x02 | ((y / 256) & 0x01);
	*(spr++) = x % 256;				/* X */
	*(spr++) = (spr_x_size / 8 - 1) * 8 | ((x / 256) & 0x03);
	/* データアドレス(TSP) */
	*(spr++) = (chr_adr / 2) % 256; //( (SPR_PAT_ADR + spr_size * chr_no) / 2) % 256;
	*(spr++) = (chr_adr / 2) / 256; // ((SPR_PAT_ADR + spr_size * chr_no) / 2) / 256;
	*(spr++) = 0;
	*(spr++) = 0;
}

unsigned char chr_tbl[8][4] = {
		{0, 1, 0 + 16, 1 + 16},
		{2, 3, 2 + 16, 3 + 16},
		{4, 5, 4 + 16, 5 + 16},
		{6, 7, 6 + 16, 7 + 16},
		{8, 9, 8 + 16, 9 + 16},
		{10, 11, 10 + 16, 11 + 16},
		{12, 13, 12 + 16, 13 + 16},
		{14, 15, 14 + 16, 15 + 16},
};

#define WIDTH 256
 //128
#define LINE 16
#define SPR_X_SIZE 32
#define SPR_Y_SIZE 16
//#define SPR_SIZE (SPR_X_SIZE / 2) * SPR_Y_SIZE
#define SPR_ATR 0x7e00

/*メインルーチン
　初期設定とメインループ*/
void main()
{
	unsigned short mode = 1, index = 0, index2 = 0, index3 = 0;
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

	unsigned char k0, k1, k8, ka, st, pd, k5, k9;
//	unsigned short index;
	unsigned char spr_no = 0;
	y = 32;

	bvram = (unsigned char __far *)MK_FP(0xa000, 0),
	rvram = (unsigned char __far *)MK_FP(0xb000, 0),
	gvram = (unsigned char __far *)MK_FP(0xc000, 0),
	ivram = (unsigned char __far *)MK_FP(0xd000, 0);
	spr_atr = (unsigned char __far *)MK_FP(0xa000, SPR_ATR);
	dir2 = 0;

	g_init();
	pal_all(pal);


	/* TSP coommand */
	while(inportb(0x142) & 0x05);
	outportb(0x142, 0x82);	/* SPRON スプライトON */
	while(inportb(0x142) & 0x05);
	outportb(0x146, (SPR_ATR) / 256);	/* スプライト制御テーブルアドレス上位 */
	while(inportb(0x142) & 0x05);
	outportb(0x146, 0x0);
	while(inportb(0x142) & 0x05);
	outportb(0x146, 32 * 4 | 0x02)	/* 横32枚 縦方向2倍 */;


	while(inportb(0x142) & 0x05);
	outportb(0x142, 0x15);	/* CURDEF */
	while(inportb(0x142) & 0x05);
	outportb(0x146, 0x02);	/* カーソルOFF */


	if((index = set_sprite_pattern("CORETEKI.SC5", SPR_PAT_ADR, 256, LINE, 32, 16, 16, 1)) == ERROR) {
		end();
		exit(1);
	}
	for(i = 0; i < 16; ++i)
		set_sprite_locate(i, SPR_PAT_ADR + SPR_SIZE(32, 16) * i, (i % 16) * 32, (i / 16) * 16, 32, 16);


	if((index2 = set_sprite_pattern("COREJIKI.SC5", SPR_PAT_ADR + index, 8 * 3 * 3, 16, 16 * 3, 16, 3, 1)) == ERROR) {
		end();
		exit(1);
	}
	for(i = 0; i < 3; ++i)
		set_sprite_locate(16+i, SPR_PAT_ADR + index + SPR_SIZE(16 * 3, 16) * i, i * 16 * 3, 64, 16 * 3, 16);

	if((index3 = set_sprite_pattern("COREBOSS.SC5", SPR_PAT_ADR + index + index2, 16*4, 32, 32 * 4, 16 * 2, 1, 1)) == ERROR) {
		end();
		exit(1);
	}
		set_sprite_locate(19, SPR_PAT_ADR + index + index2, x, y, 32 * 4, 16 * 2);

	_disable();
	outportb(0x44, 0x07);
	outportb(0x45, 0x00);
	_enable();


/* ESCで抜けるまでループ*/
	while(((k9 = inportb(0x09)) & 0x80)){ /* ESC */
		k0 = inportb(0x00);
		k1 = inportb(0x01);
		k8 = inportb(0x08);
		ka = inportb(0x0a);
		_disable();
		outportb(0x44, 0x0e);
		st = inportb(0x45);
		outportb(0x44, 0x0f);
		pd = inportb(0x45);
		_enable();
		k5 = inportb(0x05);
		if(!(k1 & 0x01) || !(k8 & 0x02) || !(st & 0x01)){ /* 8 */
//			if(y > 0){
				--y;
				dir = 0;
//			}
		}
		if(!(k0 & 0x40) || !(k8 & 0x04) || !(st & 0x08)){ /* 6 */
//			if(x < (640 - SPR_X_SIZE)){
				++x;
				dir = 1;
//			}
		}
		if(!(k0 & 0x04) || !(ka & 0x02) || !(st & 0x02)){ /* 2 */
//			if(y < (200 - SPR_Y_SIZE)){
				++y;
				dir = 2;
//			}
		}
		if(!(k0 & 0x10) || !(ka & 0x04) || !(st & 0x04)){ /* 4 */
//			if(x > 0){
				--x;
				dir = 3;
//			}
		}
		if(!(k5 & 0x04) || !(k9 & 0x40) || !(pd & 0x01)){ /* Z,SPACE */
//			++spr_no;
//			spr_no %= 32;
			if(!(k5 & 0x01) || !(pd & 0x02)) /* X */
				break;
		}
		if(!(k5 & 0x01) || !(pd & 0x02)){ /* X */
			--spr_no;
//			spr_no += 32;
//			spr_no %= 32;
		}

//		set_sprite_locate(spr_no, SPR_PAT_ADR, x, y, 32, 16);
		set_sprite_locate(19, SPR_PAT_ADR + index + index2, x, y, 32 * 4, 16 * 2);

		if((old_x != x) || (old_y != y)){
			dir2 = 1 - dir2;
		}
		old_x = x;
		old_y = y;

		while((inportb(0x0040) & 0x20));
		while(!(inportb(0x0040) & 0x20)); /* WAIT VSYNC */
	}
/*	for(i = 0; i < 32; ++i){
		set_sprite_locate(i, i, 640, 200, 0, 0);
		index = i * 8;
		spr_atr[index + 6] = 0;
		spr_atr[index + 7] = 0;
	}*/

	end();
}

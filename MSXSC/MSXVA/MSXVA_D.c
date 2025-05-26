/* MSX-SC5->PC-88VA CONV.(Digital 8 colors) for GCC-ia16 By m@3 */
/* 全体版 */

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define EOIDATA 0x20
#define EOI 0

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
#define LINE 200
#define RAM_ADR 0x2000


unsigned char __far *flame[4]
	 = {MK_FP(0xa000,0)	,MK_FP(0xb000,0),MK_FP(0xc000,0),MK_FP(0xd000,0)};

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 0, 4, 4, 1, 1, 2, 5, 2, 2, 6, 6 ,4 ,3 ,7 ,7 };

void cursor_switch(short);
void screen_switch(short);


int conv(char *loadfil)
{
	long i, j,count, count2;
	int k=0, l=0;
	unsigned char pattern[100];
	unsigned char pc88vacolor[3];
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

	for(count = 0; count < LINE; ++count){
		for(count2 = 0; count2 < WIDTH; ++count2){
	
			i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
			if(i < 1)
				break;

			/* 色分解と拡大 */
			msxcolor[0] = (pattern[0] >>4) & 0x0f;
			msxcolor[1] = (pattern[0] >>4) & 0x0f;
			msxcolor[2] = pattern[0] & 0x0f;
			msxcolor[3] = pattern[0] & 0x0f;
			msxcolor[4] = (pattern[1] >>4) & 0x0f;
			msxcolor[5] = (pattern[1] >>4) & 0x0f;
			msxcolor[6] = pattern[1] & 0x0f;
			msxcolor[7] = pattern[1] & 0x0f;
			for(i = 0; i < 3; ++i){
				pc88vacolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 3; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pc88vacolor[i]);
					}else{
						BITCLR(7-j, pc88vacolor[i]);
					}
				}
			}

			i = fread(pattern, 1, 2, stream[0]);	/* 4dot分 */
			if(i < 1)
				break;

			/* 色分解と拡大 */
			msxcolor[0] = (pattern[0] >>4) & 0x0f;
			msxcolor[1] = (pattern[0] >>4) & 0x0f;
			msxcolor[2] = pattern[0] & 0x0f;
			msxcolor[3] = pattern[0] & 0x0f;
			msxcolor[4] = (pattern[1] >>4) & 0x0f;
			msxcolor[5] = (pattern[1] >>4) & 0x0f;
			msxcolor[6] = pattern[1] & 0x0f;
			msxcolor[7] = pattern[1] & 0x0f;

			for(i = 0; i < 3; ++i){
				pattern[i] = pc88vacolor[i];
			}

			for(i = 0; i < 3; ++i){
				pc88vacolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 3; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, pc88vacolor[i]);
					}else{
						BITCLR(7-j, pc88vacolor[i]);
					}
				}
			}

			for(i = 0; i < 3; ++i){
				pattern[3 + i] = pc88vacolor[i];
			}

			*(flame[0] + 0 + k + l) = pattern[0];
			*(flame[1] + 0 + k + l) = pattern[1];
			*(flame[2] + 0 + k + l) = pattern[2];
			*(flame[3] + 0 + k + l) = 0;
			*(flame[0] + 1 + k + l) = pattern[3];
			*(flame[1] + 1 + k + l) = pattern[4];
			*(flame[2] + 1 + k + l) = pattern[5];
			*(flame[3] + 1 + k + l) = 0;
/*			*(flame[0] + 80 + k + l) = pattern[0];
			*(flame[1] + 80 + k + l) = pattern[1];
			*(flame[2] + 80 + k + l) = pattern[2];
			*(flame[3] + 80 + k + l) = 0;
			*(flame[0] + 81 + k + l) = pattern[3];
			*(flame[1] + 81 + k + l) = pattern[4];
			*(flame[2] + 81 + k + l) = pattern[5];
			*(flame[3] + 81 + k + l) = 0;
*/
			k += 2;
			if(k >= 64){
				k = 0;
				l += 80 * 1;
			}
		}
	}
	fclose(stream[0]);

	return 0;
}

void g_init(void)
{
//	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
//	outportw(0x100, 0xb020);	/* none-interless Graphic-on 200dot(200line) */
	outportw(0x100, 0xb062);	/* none-interless Graphic-on 400dot(200line) */
								/* 画面ON notsingle-plane 1画面 */

	outportw(0x102, 0x0101);	/* graphic0 Width640 4dot/pixel */
								/* graphic1 Width640 4dot/pixel */

	outportb(0x153, 0x44);		/* G-VRAM選択 */

	outportw(0x106, 0xab90);	/* パレット指定画面割当て指定 */
	outportw(0x108, 0x0000);	/* 直接色指定画面割当て設定 */
	outportw(0x110, 0x008f);	/* 4ビットピクセル */
//	outportb(0x500, 0);	/* 独立アクセス */
//	outportb(0x512, 0);	/* ブロック0 */
//	outportb(0x516, 0);	/* 書き込みプレーン選択 */
}

/*終了処理*/
void end()
{
	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
	outportb(0x153, 0x41);		/* T-VRAM選択 */
	outportw(0x106, 0xab98);	/* パレット指定画面割当て指定 */
}

/*カーソル及びファンクションキー表示の制御*/
/*void cursor_switch(short mode)
{
	if(mode)
		printf("\x1b[>1l\x1b[>5l");
	else
		printf("\x1b*\x1b[>1h\x1b[>5h");
}*/

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
void clear(short type)
{
	if(type & 1)
		paint(0x0);

//	if(type & 2)
//		printf("\x1b*");
}

/*ページ切り替え*/
/*void setpage(short visual, short active)
{
	outportb(0xa4, visual);
	outportb(0xa6, active);
}*/

/*パレット・セット*/
void pal_set(unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outportw(0x300 + color * 2, (unsigned short)green * 4096 + red * 64 + blue * 2);
}

void pal_all(unsigned char color[16][3])
{
	short i;
	for(i = 0; i < 16; i++)
		pal_set(i, color[i][0], color[i][1], color[i][2]);
}


int	main(int argc,char **argv){

	short i,j;
	unsigned short a, b;

	unsigned char color[16][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },
	{ 0, 0, 0 },};

//	screen_switch(OFF);

	if (argv[1] == NULL){
		printf("MSX .SC5 file Converter for PC-88VA.\n");
		return 1;
	}

	g_init();
	pal_all(color);
//	setpage(0,0);
//	screen_switch(ON);


	clear(3);
	conv(argv[1]);

	end();

	return 0;
}

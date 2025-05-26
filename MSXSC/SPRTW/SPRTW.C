/* MSX-SC5->FM TOWNS SPR CONV. for High-C */

#include <stdio.h>
#include <stdlib.h>

#include <egb.h>
#include <snd.h>
#include <spr.h>
#include <dos.h>
#include <conio.h>

char egb_work[1536];

#define PUT_SP( x, y, no, atr) {\
	*(spram++) = x; \
	*(spram++) = y; \
	*(spram++) = no; \
	*(spram++) = atr; \
}

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


#define MSXWIDTH 256
#define MSXLINE 212
#define SPRSIZEX 8
#define SPRSIZEY 16
#define SPRPARTS 256

#define MAX_SPRITE 256

FILE *stream[2];

unsigned char pattern[10];
unsigned char msxcolor[MSXWIDTH / 2][MSXLINE];

int conv(char *loadfil)
{

	long i, j,k,y, x, xx, yy, no, max_xx;

	_Far unsigned short *spram;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}

	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */
	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */

	for(y = 0; y < MSXLINE; ++y){
		for(x = 0; x < MSXWIDTH / 2 ; ++x){
			msxcolor[x][y] = 0;
		}
	}
	for(y = 0; y < MSXLINE; ++y){
		for(x = 0; x < MSXWIDTH / 8; ++x){
			i = fread(pattern, 1, 4, stream[0]);	/* 8dot分 */
			if(i < 1)
				break;

			/* 色分解 */
			msxcolor[0 + x * 4][y] =
				((pattern[1] >>4) & 0x0f) | ((pattern[1] & 0x0f) * 16); 
			msxcolor[1 + x * 4][y] =
				((pattern[0] >>4) & 0x0f) | ((pattern[0] & 0x0f) * 16); 
			msxcolor[2 + x * 4][y] =
				((pattern[3] >>4) & 0x0f) | ((pattern[3] & 0x0f) * 16); 
			msxcolor[3 + x * 4][y] =
				((pattern[2] >>4) & 0x0f) | ((pattern[2] & 0x0f) * 16); 
		}
	}
	fclose(stream[0]);
	max_xx = 128;

	_FP_SEG(spram) = 0x130;
	_FP_OFF(spram) = 0x4000;
	j = 0;
	xx=0;
	yy=0;
	x=0;
	for(no = 0; no < SPRPARTS; ++no){
//		printf("\nno =%d ",no);
		for(y = 0; y < SPRSIZEY; ++y){
			for(x = 0; x < SPRSIZEX; x+=2){

				if((x+xx) >= max_xx) {
					xx=0;
					yy+=SPRSIZEY;
				}

				*(spram++) = msxcolor[x + xx][y + yy] * 256 + msxcolor[x + xx + 1][y + yy];
			}
		}
		xx+=SPRSIZEX;
	}

	return 0;
}

void g_init(void)
{
//	char para[64];

	EGB_init(egb_work, 1536);

/* 31kHz出力用 */
	EGB_resolution(egb_work, 0, 10);		/* ペ－ジ0は512x256/32768 */
	EGB_resolution(egb_work, 1, 5);			/* ペ－ジ1は256x512/32768 */
	EGB_displayPage(egb_work, 1, 3);		/* 上にくるペ－ジは1で両方とも表示 */
	EGB_writePage(egb_work, 0);				/* ペ－ジ0(BG)の設定 */
	EGB_displayStart(egb_work, 2, 2, 2);		/* 表示拡大率(縦横2倍) */
	EGB_displayStart(egb_work, 3, 256, 240);	/* EGB画面の大きさ(256x240) */
	EGB_displayStart(egb_work, 0, 32, 0);		/* 表示開始位置 */
	EGB_displayStart(egb_work, 1, 0, 0);		/* 仮想画面中の移動 */

	EGB_writePage(egb_work, 1);				/* ペ－ジ1(スプライト)の設定 */
	EGB_displayStart(egb_work, 2, 2, 2);		/* */
	EGB_displayStart(egb_work, 3, 256, 240);	/* */
	EGB_displayStart(egb_work, 0, 32, 0);		/* */
	EGB_displayStart(egb_work, 1, 0, 2);		/* 下に2ドットずらす(仕様) */

	EGB_color(egb_work, 0, 0x8000);				/* ペ－ジ1をクリアスクリ－ン */
	EGB_color(egb_work, 2, 0x8000);				/* 透明色で埋める */
	EGB_writePage(egb_work, 1);
	EGB_clearScreen(egb_work);

}

/*終了処理*/
void end()
{
	EGB_resolution(egb_work, 0, 4);		/* ペ－ジ0は640x400/16 */
	EGB_resolution(egb_work, 1, 4);		/* ペ－ジ1は640x400/16 */
	EGB_displayPage(egb_work, 0, 3);

	EGB_writePage(egb_work, 0);			/* ペ－ジ0をクリアスクリ－ン */
	EGB_clearScreen(egb_work);
	EGB_displayStart(egb_work,0,0,0);
	EGB_displayStart(egb_work, 1, 0, 0);
	EGB_displayStart(egb_work,2,1,1);
	EGB_displayStart(egb_work, 3, 640, 400);
	EGB_writePage(egb_work, 1);			/* ペ－ジ1をクリアスクリ－ン */
	EGB_clearScreen(egb_work);
	EGB_displayStart(egb_work,0,0,0);
	EGB_displayStart(egb_work, 1, 0, 0);
	EGB_displayStart(egb_work,2,1,1);
	EGB_displayStart(egb_work, 3, 640, 400);
}

/*パレット・セット*/
void pal_set(unsigned char color, unsigned char red, unsigned char blue,
	unsigned char green)
{
	_Far unsigned short *palram;

	_FP_SEG(palram) = 0x130;
	_FP_OFF(palram) = 0x2000;

	palram[color] = green * 32 * 32 + red * 32 + blue;
}

void pal_all(void)
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(i, ((pal[i][0] + 1)*2-1) * (pal[i][0] != 0), ((pal[i][2]+1)*2-1) * (pal[i][2] != 0), ((pal[i][1]+1)*2-1) * (pal[i][1] != 0));
}

int	main(int argc,char **argv){

	_Far unsigned short *spram, *vram;
	unsigned short i;

	if (argv[1] == NULL)
		return 1;

	g_init();
	SPR_init();
	_FP_SEG(vram)=0x120;

/* バッファ0 */
	_FP_OFF(vram) = 0x40000;
	i = 512;
	while(i--)
		*vram++ = 0x8000;

/* バッファ1 */
	_FP_OFF(vram) = 0x60000;
	i = 512;
	while(i--)
		*vram++ = 0x8000;
	SPR_display(1, MAX_SPRITE);

	pal_all();

	_FP_SEG(spram)=0x130;
	_FP_OFF(spram) = (1024 - MAX_SPRITE) * 8;

	for(i = 0; i <  MAX_SPRITE; i++){
		PUT_SP((i % 16) * 16, (i / 16) * 16 + 2, i+128, 256 | 0x8000);
	}

	conv(argv[1]);

	getch();

	SPR_display(0,1);
	end();

	exit(0);

	return 0;
}

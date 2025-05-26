/* MSX-SC5->FM TOWNS CONV.(Analog 16 colors) for HighC 386 By m@3 */
/* 32768色版 */

#include <stdio.h>
#include <stdlib.h>

#include <egb.h>
#include <snd.h>
#include <spr.h>
#include <dos.h>
#include <conio.h>

char egb_work[1536];

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

#define WIDTH 32
#define LINE 212

_Far short *vram_adr;

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};


int conv(char *loadfil)
{
	long i, count, count2;
	int k=0, l=0, m=0;
	unsigned char read_pattern[WIDTH * LINE * 2+ 2];
	unsigned char pattern[10];
	unsigned short fmtcolor[4];
	unsigned char msxcolor[8];
	unsigned char color;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX先頭を読み捨てる */
	fread(pattern, 1, 4, stream[0]);	/* MSXヘッダも読み捨てる */

	fread(pattern, 1, 2, stream[0]);	/* MSXヘッダを読み捨てる */


	for(count = 0; count < 4; ++count){
		i = fread(read_pattern, 1, WIDTH * LINE, stream[0]);
		m = 0;
//		if(i < 1)
//			break;
		for(count2 = 0; count2 < WIDTH * LINE; ++count2){
	

			/* 色分解 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = read_pattern[m++] & 0x0f;

			color = msxcolor[0];

			fmtcolor[0] = 
				(((pal[color][1] + 1) * 2 - 1) * (pal[color][1] != 0)) * 32 * 32 + 
				(((pal[color][0] + 1) * 2 - 1) * (pal[color][0] != 0)) * 32 + 
				((pal[color][2] + 1) * 2 - 1) * (pal[color][2] != 0);

			color = msxcolor[1];
			fmtcolor[1] = 
				(((pal[color][1] + 1) * 2 - 1) * (pal[color][1] != 0)) * 32 * 32 + 
				(((pal[color][0] + 1) * 2 - 1) * (pal[color][0] != 0)) * 32 + 
				((pal[color][2] + 1) * 2 - 1) * (pal[color][2] != 0);

			_FP_OFF(vram_adr) = (k + l) * 4;
			*vram_adr = fmtcolor[0];
			vram_adr++;
			*vram_adr = fmtcolor[1];

			k += 1;
			if(k >= (128)){
				k = 0;
				l += (256);
			}
		}
	}
	fclose(stream[0]);

	return 0;
}

void g_init(void)
{
//	char para[64];

	EGB_init(egb_work, 1536);

/* 31kHz出力用 */
	EGB_resolution(egb_work, 0, 10);		/* ペ－ジ0は512x256/32768 */
	EGB_resolution(egb_work, 1, 5);			/* ペ－ジ1は256x512/32768 */
	EGB_displayPage(egb_work, 0, 3);		/* 上にくるペ－ジは0で両方とも表示 */

/* もし15kHz出力したいならこうする */
/* 	EGB_resolution(egb_work, 0, 8);			/* ペ－ジ0は512x256/32768 */
/* 	EGB_resolution(egb_work, 1, 11);		/* ペ－ジ1は256x512/32768 */

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

void paint(unsigned short color)
{
	unsigned short i, j;

	for (i = 0; i < (256); ++i){
		for (j = 0; j < 256; ++j){
			_FP_OFF(vram_adr) = (j + i * 512) * 2;
			*vram_adr = color; /* color */;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1){
		paint(0x0);
	}
	if(type & 2)
		printf("\x1b*");
}

int	main(int argc,char **argv){

	if (argv[1] == NULL){
		printf("MSX .SC5 file Converter for FM TOWNS.\n");
		getch();
		return 1;
	}

	g_init();

	_FP_SEG(vram_adr)=0x120;
	clear(3);
	conv(argv[1]);

	getch();
	end();

	return 0;
}

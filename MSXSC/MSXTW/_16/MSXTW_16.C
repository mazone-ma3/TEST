/* MSX-SC5->FM TOWNS CONV.(Analog 16 colors) for HighC 386 By m@3 */
/* 16色版 */

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
		for(count2 = 0; count2 < WIDTH * LINE / 2; ++count2){
	

			/* 色分解 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = read_pattern[m++] & 0x0f;

			fmtcolor[0] = msxcolor[0] + msxcolor[1] * 16;
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = read_pattern[m++] & 0x0f;
			fmtcolor[1] = msxcolor[0] + msxcolor[1] * 16;

			_FP_OFF(vram_adr) = k + l * 2;
			*vram_adr = fmtcolor[0] + fmtcolor[1] * 256;
			vram_adr++;

			k += 2;
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
//	EGB_resolution(egb_work, 0, 10);		/* ペ－ジ0は512x256/32768 */
	EGB_resolution(egb_work, 0, 3);			/* ペ－ジ0は640x480/16 */
	EGB_resolution(egb_work, 1, 5);			/* ペ－ジ1は256x512/32768 */
	EGB_displayPage(egb_work, 0, 3);		/* 上にくるペ－ジは0で両方とも表示 */
	EGB_writePage(egb_work, 0);				/* ペ－ジ0(BG)の設定 */
	EGB_displayStart(egb_work, 2, 2, 2);		/* 表示拡大率(縦横2倍) */
	EGB_displayStart(egb_work, 3, 640, 480);	/* EGB画面の大きさ(640x480) */
	EGB_displayStart(egb_work, 0, 0, 0);		/* 表示開始位置 */
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

	for (i = 0; i < (480); ++i){
		for (j = 0; j < 640; ++j){
			_FP_OFF(vram_adr) = (j + i * 640) * 1;
			*vram_adr = color;
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

/*パレット・セット*/
void pal_set(unsigned char color, unsigned char red, unsigned char blue,
	unsigned char green)
{
	_outp(0x448,0x01);
	_outp(0x44a,0x00);	/* priority register */

	_outp(0xfd90, color);
	_outp(0xfd92, blue * 16);
	_outp(0xfd94, red * 16);
	_outp(0xfd96, green * 16);
}

void pal_all(void)
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(i, ((pal[i][0] + 1)*1-1) * (pal[i][0] != 0), ((pal[i][2]+1)*1-1) * (pal[i][2] != 0), ((pal[i][1]+1)*1-1) * (pal[i][1] != 0));
}

int	main(int argc,char **argv){

	if (argv[1] == NULL){
		printf("MSX .SC5 file Converter for FM TOWNS.\n");
		getch();
		return 1;
	}

	g_init();
	pal_all();

	_FP_SEG(vram_adr)=0x120;
	clear(3);

	conv(argv[1]);

	getch();
	end();

	return 0;
}

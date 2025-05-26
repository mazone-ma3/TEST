/* MSX-SC5->X68K-PCG/SPR CONV. for GCC */

#include <stdio.h>
#include <stdlib.h>
#include <sys\dos.h>
#include <conio.h>

#include <doslib.h>
#include <iocslib.h>

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
#define PCGSIZEX 4
#define PCGSIZEY 8
#define PCGPARTS 256
#define MAXSPRITE 128

FILE *stream[2];

unsigned char pattern[10];
unsigned char msxcolor[MSXWIDTH / 2][MSXLINE];

int conv(char *loadfil)
{
	long i, j,k,y, x, xx, yy, no, max_xx;

	unsigned short *spram;

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
			msxcolor[0 + x * 4][y] = pattern[0]; 
			msxcolor[1 + x * 4][y] = pattern[1]; 
			msxcolor[2 + x * 4][y] = pattern[2];
			msxcolor[3 + x * 4][y] = pattern[3];
		}
	}
	fclose(stream[0]);
	max_xx = 128;
	spram  = (unsigned short *)0xeb8000;

	j = 0;
	xx=0;
	yy=0;
	x=0;
	for(no = 0; no < PCGPARTS; ++no){
		for(i = 0; i < 2; ++i){
			for(j = 0; j < 2; ++j){
//				printf("\nno =%d ",no);
				for(y = 0; y < PCGSIZEY; ++y){
					for(x = 0; x < PCGSIZEX; x+=2){

						if((x+xx) >= max_xx) {
							xx=0;
							yy+=PCGSIZEY*2;
						}

						*(spram++) = msxcolor[x + xx][y + yy] * 256 + msxcolor[x + xx + 1][y + yy];
					}
				}
				yy+=PCGSIZEY;
			}
			yy-=PCGSIZEY*2;
			xx+=PCGSIZEX;
		}
	}

	return 0;
}

void g_init(void)
{
/*	CRTMOD(0x0a); */ 	/* 256x256 256colors 2plane 31kHz */
	CRTMOD(0x06);	/* 256x256 16colors 4plane 31kHz */
//	CRTMOD(0x0e);	/* 256x256 65536colors 1plane 31kHz */
	G_CLR_ON();
	B_CUROFF();
}

/*終了処理*/
void end()
{
	B_CURON();
	CRTMOD(0x10);	/* 768x512 16colors 1plane 31kHz */
}
/*パレット・セット*/
void pal_set(unsigned char color, unsigned char red, unsigned char blue,
	unsigned char green)
{
	unsigned short *pal_port;
	pal_port = (unsigned short *)(0xe82200 + color * 2);
	*pal_port = (green * 32 * 32 + red * 32 + blue) * 2 + 1;
}

void pal_all(void)
{
	unsigned char i;
	for(i = 0; i < 16; i++)
		pal_set(i, ((pal[i][0] + 1)*2-1) * (pal[i][0] != 0), ((pal[i][2]+1)*2-1) * (pal[i][2] != 0), ((pal[i][1]+1)*2-1) * (pal[i][1] != 0));
}

int	main(int argc,char **argv){

	unsigned short *spram, i;

	if (argv[1] == NULL)
		return 1;

dum:	B_SUPER(0);		/* スーパーバイザモード 最適化防止にラベルを付ける */
	g_init();
	pal_all();

	SP_ON();

	spram = (unsigned short *)0xeb0000;

	for(i = 0; i < MAXSPRITE; i++){
		PUT_SP((i % 16) * 16 + 16, (i / 16) * 16 + 16, i, 0x0011);
	}

	conv(argv[1]);

	getch();

	SP_OFF();
	end();

	exit(0);

	return 0;
}

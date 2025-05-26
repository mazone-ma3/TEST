/* MSX-SC5->X68K CONV.(Analog 16 colors) for GCC By m@3 */
/* 16色(Text)版 */

#include <stdio.h>
#include <stdlib.h>
#include <sys\dos.h>
#include <conio.h>

#include <doslib.h>
#include <iocslib.h>

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

unsigned char *vram_adr;

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 , 15};

int conv(char *loadfil)
{
	long i, j, count, count2;
	int k=0, l=0, m=0;
	unsigned char read_pattern[WIDTH * LINE * 2+ 2];
	unsigned char pattern[10];
	unsigned short x68color[4];
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
		for(count2 = 0; count2 < WIDTH * LINE / 4; ++count2){
	
			/* 色分解 */
			msxcolor[0] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[1] = read_pattern[m++] & 0x0f;
			msxcolor[2] = (read_pattern[m] >> 4) & 0x0f;
			msxcolor[3] = read_pattern[m++] & 0x0f;
			msxcolor[4] = (read_pattern[m] >>4) & 0x0f;
			msxcolor[5] = read_pattern[m++] & 0x0f;
			msxcolor[6] = (read_pattern[m] >> 4) & 0x0f;
			msxcolor[7] = read_pattern[m++] & 0x0f;

			for(i = 0; i < 4; ++i){
				x68color[i] = 0;
			}
			for(j = 0; j < 8; ++j){
				for(i = 0; i < 4; ++i){
					color = conv_tbl[msxcolor[j]];	/* 色変換 */
					if(BITTST(i, color)){
						BITSET(7-j, x68color[i]);
					}else{
						BITCLR(7-j, x68color[i]);
					}
				}
			}
			for(i = 0; i < 4; ++i){
				pattern[i] = x68color[i];
			}
			vram_adr = (unsigned char *)0xe00000 + k + l; // * 2;
			*(vram_adr + 0x20000 * 0) = pattern[0];
			*(vram_adr + 0x20000 * 1) = pattern[1];
			*(vram_adr + 0x20000 * 2) = pattern[2];
			*(vram_adr + 0x20000 * 3) = pattern[3];

			k += 1;
			if(k >= (32)){
				k = 0;
				l += 0x80; //(256);
			}
		}
	}
	fclose(stream[0]);

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

void paint(unsigned short color)
{
	unsigned short i, j;

	for (i = 0; i < 512; ++i){
		for (j = 0; j < 0x80; ++j){
			*(vram_adr + j + i * 0x80 + 0x20000 * 0) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 1) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 2) = color; /* bit */;
			*(vram_adr + j + i * 0x80 + 0x20000 * 3) = color; /* bit */;
		}
	}
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1)
		paint(0x0);

	if(type & 2)
		printf("\x1b*");
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

	if (argv[1] == NULL){
		printf("MSX .SC5 file Converter for X68K.\n");
		return 1;
	}
	vram_adr = (unsigned char *)0xe00000;
dum:	B_SUPER(0);		/* スーパーバイザモード 最適化防止にラベルを付ける */
	g_init();
	pal_all();

	clear(3);

	conv(argv[1]);

	getch();
	end();

	exit(0);

	return 0;
}

//starfall for gcc-ia16 & libi86 & PC-98 2022/2/21 By m@3

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

//#pragma check_stack(off)
//#pragma check_poshorter(off)

#define EOIDATA 0x20
#define EOI 0

#define ON 1
#define OFF 0
#define PAT 1

void __interrupt __far v_sync(void);
void __interrupt __far(*keepvector)(void);

unsigned char vs_count = 0,	old_vs_count = 0;
unsigned char keepport;

unsigned char __far *flame[4]
	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};
unsigned short star_y[80],star_inc[80],star_color[80];

void cursor_switch(short);
void screen_switch(short);
void clear(short);


void __interrupt __far  v_sync(void)
{
	++vs_count;
}

void init_v_sync(void)
{
	_disable();
	keepport = inportb(2);
	keepvector = _dos_getvect(10);
	_dos_setvect(10, v_sync);


	outportb(EOI, EOIDATA);
	outportw(2,inportb(2) & 0xfb);

	outportw(0x64, 1);	/* VSYNC初期化 */
	_enable();
}

void term_v_sync(void)
{
	_disable();
	_dos_setvect(10, keepvector);
	outportb(2, keepport);
	_enable();
}

void g_init(void)
{
	_disable();
	outportb(0x6a, 1   ); /* 16色モード (0x6a=mode f/fp2)*/
	outportb(0xa2, 0x4b);
	outportb(0xa0, 0); /* L/R = 1 (縦方向の拡大係数)*/
	outportb(0x68, 8   ); /* モードF/F1 (8で高解像度)*/
	outportw(0x4a0,0xfff0);
	outportw(0x7c, 0); //
	_enable();
	cursor_switch(OFF);

}

/*終了処理*/
void end()
{
	cursor_switch(ON);
}

/*カーソル及びファンクションキー表示の制御*/
void cursor_switch(short mode)
{
	if(mode)
		printf("\x1b[>1l\x1b[>5l");
	else
		printf("\x1b*\x1b[>1h\x1b[>5h");
}

void screen_switch(short mode)
{
	if(mode)
		outportb(0xa2, 0x0d); /* 表示開始 */
	else
		outportb(0xa2, 0x0c);
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	unsigned short i,j;

	if(type & 1){
		for (i = 0; i < 80 * 400; i++)
			for(j = 0; j < 4; j++)
				*(flame[j] + i) = 0;

	}

	if(type & 2)
		printf("\x1b*");
}

/*ページ切り替え*/
void setpage(short visual, short active)
{
	outportb(0xa4, visual);
	outportb(0xa6, active);
}

/*パレット・セット*/
void pal_set(short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
		outportw(0xa8, color);
		outportw(0xaa, green);
		outportw(0xac, red);
		outportw(0xae, blue);
}

void pal_all(unsigned char color[16][3])
{
	short i;
	for(i = 0; i < 15; i++)
		pal_set(i, color[i][0], color[i][1], color[i][2]);
}

short main(void)
{
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

	screen_switch(OFF);
	g_init();
	pal_all(color);
	setpage(0,0);
	screen_switch(ON);

	for(i = 0; i < 80; i++){
		star_y[i] = rand() % 50 * 8;
		star_inc[i] = rand() % 3 + 1;
		star_color[i] = rand() % 6 + 1;
	}
	clear(3);
	outportb(0x62,0x0c);	/**Text OFF */

	init_v_sync();
	while(kbhit() == 0){
		if(vs_count){
			vs_count=0;
			for(i = 0; i < 80; i++){
				a = i + star_y[i] * 80;
				b = a + star_inc[i] * 80;
				star_y[i] += star_inc[i];
				star_y[i] %= 400;

				for(j = 0; j < 3; j++){
					if((star_color[i] & (1 << j)) != 0){
						*(flame[j] + a) &= (~PAT) ;
						*(flame[j] + b) |= PAT;
					}
				}
			}
			outportw(EOI, EOIDATA);
			outportw(0x64, 1);	/* VSYNC初期化 */
		}
	}
	term_v_sync();

	clear(3);
	outportb(0x62,0x0d);	/**Text ON */
	end();

	return 0;
}
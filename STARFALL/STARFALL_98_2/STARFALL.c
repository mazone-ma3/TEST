//starfall for gcc-ia16 & libi86 & PC-98 By m@3

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

__far __interrupt void (*keepvector)(void);

volatile unsigned char vs_count = 0;
unsigned char keepport;

#define VECT 0x0a

__far volatile unsigned char *flame[4]
	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};

 unsigned short star_y[80],star_inc[80],star_color[80];

void cursor_switch(short);
void screen_switch(short);
void clear(short);

/*垂直同期待ち*/
/*void wait_vsync(void)
{
	while(!(inportb(0x60) & 0x20));
	while((inportb(0x60) & 0x20));
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}*/

__far __interrupt void ip_v_sync(void)
{
	++vs_count;
//	outportb(EOI, EOIDATA);
__asm volatile ( " movb $0x20,%al");
__asm volatile ( " outb %al,$0x00");
//	outportb(0x64, 1);	/* VSYNC初期化 */
__asm volatile ( " movb $0x01,%al");
__asm volatile ( " outb %al,$0x64"); //%dx");
}

void init_v_sync(void)
{
	_disable();
	keepport = inportb(2);
	keepvector = _dos_getvect(VECT);
	_dos_setvect(VECT, ip_v_sync);

	outportb(EOI, EOIDATA);
	outportb(2, keepport & 0xfb);

	outportb(0x64, 1);	/* VSYNC初期化 */
	_enable();
}

void term_v_sync(void)
{
	_disable();
	_dos_setvect(VECT, keepvector);
	outportb(2, keepport);
	_enable();
}

/*タイマウェイト*/
/*void wait(unsigned short wait)
{
	while(vs_count < wait);
}*/

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

volatile unsigned short a, b, i, j;

#define ESC    (key_scan(0x0) & 0x01)

union REGS reg;
union REGS reg_out;

unsigned char key_scan(unsigned short group)
{
	reg.h.ah = 0x04;
	reg.h.al = (unsigned char)group;
	int86(0x18, &reg, &reg);
	return(reg.h.ah);
}

short main(void)
{
//	short i,j;

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
	while(1){
//		_disable();
		vs_count = 0;
//		_enable();

		for(i = 0; i < 80; i++){
			star_y[i] += star_inc[i];
			star_y[i] %= 400;
			a = i + star_y[i] * 80;
			b = a + star_inc[i] * 80;

			for(j = 0; j < 3; j++){
				if((star_color[i] & (1 << j)) != 0){
//					_disable();
					*(flame[j] + a) &= (~PAT) ;
					if(b < 400 * 80)
						*(flame[j] + b) |= PAT;
//					_enable();
				}
			}
		}

//		wait(2);
		for(;;){
//			if(kbhit() != 0)
			_disable();
			if(ESC)
				goto end;
			if(vs_count >= 1){
				break;
			}
			_enable();
		}
//		sys_wait(1);
		_enable();
	}
end:
//	_enable();
	term_v_sync();

	clear(3);
	outportb(0x62,0x0d);	/**Text ON */
	end();

	return 0;
}
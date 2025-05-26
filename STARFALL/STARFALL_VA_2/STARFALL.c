//starfall for gcc-ia16 & libi86 & PC-88VA  By m@3

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define EOIDATA 0x20
#define EOI 0x188

#define ON 1
#define OFF 0
#define PAT 1

#define cli() __asm volatile ( " cli");
#define sti() __asm volatile ( " sti");


__far __interrupt void (*keepvector)(void);

__far unsigned char vs_count = 0;
unsigned char keepport;

#define VECT 0x0a

__far volatile unsigned char *flame[4]
	 = {MK_FP(0xa000,0)	,MK_FP(0xb000,0),MK_FP(0xc000,0),MK_FP(0xd000,0)};

unsigned short star_y[80],star_inc[80],star_color[80];

union REGS reg;
union REGS reg_out;


/*垂直同期待ち*/
void wait_vsync(void)
{
	while(!(inportb(0x0040) & 0x20));
	while((inportb(0x0040) & 0x20));
}

void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}

__far __interrupt void ip_v_sync(void)
{
//	cli();
//	_disable();
//	vs_count = 1;
	++vs_count;
//	outportb(EOI, EOIDATA);
__asm volatile ( " movb $0x20,%al");
__asm volatile ( " movw $0x188,%dx");
__asm volatile ( " outb %al,%dx"); //%dx");
}

void init_v_sync(void)
{
	_disable();
	keepport = inportb(0x18a);
	keepvector = _dos_getvect(VECT);
	_dos_setvect(VECT, ip_v_sync);
	outportb(0x18a, keepport & 0xfb);

	_enable();
}

void term_v_sync(void)
{
	_disable();
	_dos_setvect(VECT, keepvector);
	outportb(0x18a, keepport);
	_enable();
}

/*タイマウェイト*/
/*void wait(unsigned short wait)
{
	while(vs_count < wait);
}*/

void g_init(void)
{
	union REGS reg;
	union REGS reg_out;

	reg.h.ah = 0x2a;
	int86(0x83, &reg, &reg_out);	/* テキスト初期化 */

	reg.h.ah = 0x00;
	reg.x.bx = 0x2000;
	reg.h.cl=4;
	reg.h.ch=4;
	int86(0x8f, &reg, &reg_out);	/* グラフィックBIOS初期化 */

//	outportw(0x100, 0xb060);	/* none-interless Graphic-on 400dot(400line) */
//	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
//	outportw(0x100, 0xb020);	/* none-interless Graphic-on 200dot(200line) */
//	outportw(0x100, 0xb062);	/* none-interless Graphic-on 400dot(200line) */
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
	outportb(0x153, 0x41);	/* T-VRAM選択 */
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

	for (i = 0; i < (80 * 400L); ++i){
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

volatile unsigned short a, b, i, j;

int main(void)
{
	g_init();

	for(i = 0; i < 80; i++){
		star_y[i] = rand() % 50 * 8;
		star_inc[i] = rand() % 3 + 1;
		star_color[i] = rand() % 6 + 1;
	}

	clear(3);

	init_v_sync();
	while(1){
//		_disable();
		cli();
		vs_count = 0;
//		outportb(EOI, EOIDATA);
//__asm volatile ( " movb $0x20,%al");
//__asm volatile ( " movw $0x188,%dx");
//__asm volatile ( " outb %al,%dx"); //%dx");
//		_enable();
		sti();

		for(i = 0; i < 80; i++){
			star_y[i] += star_inc[i];
			star_y[i] %= 400;
			a = i + star_y[i] * 80;
			b = a + star_inc[i] * 80;

			for(j = 0; j < 3; j++){
				if((star_color[i] & (1 << j)) != 0){
//					_disable();
					cli();
					*(flame[j] + a) &= (~PAT) ;
					if(b < 400 * 80)
						*(flame[j] + b) |= PAT;
					sti();
//					_enable();
				}
			}
		}
//		wait(2);
		for(;;){
//			_disable();
			cli();
//			if(!((inportb(0x09)) & 0x80)) /* ESC */
__asm volatile ( " inb $0x09, %al"); //%dx");
__asm volatile ( " nop" :"=a"(i));
			if(!(i & 0x80))
				goto end;
			if(vs_count >= 1){
				break;
			}
//			_enable();
			sti();
__asm volatile ( " nop");
		}
//		sys_wait(1);
//		_enable();
		sti();
	}
end:
//	_enable();
	term_v_sync();

//	clear(3);

	end();

	return 0;
}

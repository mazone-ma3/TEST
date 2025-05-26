//starfall for gcc-ia16 & libi86 & PC-AT 2022/3/18 By m@3

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

//#pragma check_stack(off)
//#pragma check_poshorter(off)

#define ON 1
#define OFF 0
#define PAT 1

unsigned char __far *vram_adr = MK_FP(0xa000, 0);

unsigned short star_y[80],star_inc[80],star_color[80];

void cursor_switch(short);
void screen_switch(short);
void clear(short);


union REGS reg;
union REGS reg_out;


unsigned char old_mode;

void g_init(void)
{
	reg.h.ah = 0x0f;
	int86(0x10, &reg, &reg);
	old_mode = reg.h.al;

	reg.h.ah = 0x00;
	reg.h.al = 0x12;	/* VGA mode 640x480/16colors */
	int86(0x10, &reg, &reg);
}

/*終了処理*/
void end()
{
	reg.h.ah = 0x00;
	reg.h.al = old_mode;
	int86(0x10, &reg, &reg);
}

/*カーソル及びファンクションキー表示の制御*/
void cursor_switch(short mode)
{
	if(mode)
		printf("\x1b[>1l\x1b[>5l");
	else
		printf("\x1b*\x1b[>1h\x1b[>5h");
}

void paint(unsigned char plane, unsigned char color)
{
	unsigned short i;

	outportb(0x3c4,0x02);
	outportb(0x3c5,plane);	/* 各プレーンへの書き込み有効 */

	for (i = 0; i < (80 * 480L); ++i)
		*(vram_adr + i) = color;
}

/*テキスト画面及びグラフィック画面の消去*/
void clear(short type)
{
	if(type & 1)
		paint(0x0f, 0);

	if(type & 2)
		printf("\x1b*");
}


short main(void)
{
	unsigned short i, j;
	unsigned short a, b;
//	unsigned char old_vga_mode = 0;

	vram_adr = MK_FP(0xa000, 0);

	g_init();

	for(i = 0; i < 80; i++){
		star_y[i] = rand() % 50 * 8;
		star_inc[i] = rand() % 3 + 1;
		star_color[i] = rand() % 6 + 1;
	}

//	outportb(0x3c2,0xc3);
//	outportb(0x3c3,0x01);	/* VGA ON */

//	outportb(0x3c4,0x02);
//	outportb(0x3c5,0x0f);	/* 各プレーンへの書き込み有効 */

//	outportb(0x3ce,0x06);
//	old_vga_mode = inportb(0x3cf);
//	outportb(0x3ce,0x06);	/* VGA mode Set */
//	outportb(0x3cf,0x01);

	clear(3);

	while(kbhit() == 0){
		while((inportb(0x3da) & 0x08));
		while(!(inportb(0x3da) & 0x08)); /* WAIT VSYNC */

		for(i = 0; i < 80; i++){
			a = i + star_y[i] * 80;
			b = a + star_inc[i] * 80;
			star_y[i] += star_inc[i];
			star_y[i] %= 480;

			outportb(0x3ce, 0x05);	// write mode
			outportb(0x3cf, 0x00);
			outportb(0x3ce, 0x08);	// bit
			outportb(0x3cf, 0xff);

			outportb(0x3c4, 0x02);
			outportb(0x3c5, star_color[i]);
			*(vram_adr + a) &= (~PAT);
			*(vram_adr + b) |= PAT;
		}

	}

	clear(3);
//	outportb(0x3ce,0x06);
//	outportb(0x3cf,old_vga_mode);

	end();

	return 0;
}
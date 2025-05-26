//starfall for gcc-ia16 & libi86 & PC-88VA  By m@3

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define ON 1
#define OFF 0
#define PAT 1

unsigned char __far *flame[4]
	 = {MK_FP(0xa000,0)	,MK_FP(0xb000,0),MK_FP(0xc000,0),MK_FP(0xd000,0)};

unsigned short star_y[80],star_inc[80],star_color[80];

union REGS reg;
union REGS reg_out;


unsigned char old_mode;

void g_init(void)
{
//	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
//	outportw(0x100, 0xb020);	/* none-interless Graphic-on 200dot(200line) */
	outportw(0x100, 0xb042);	/* none-interless Graphic-on 400dot(200line) */
								/* ���ON notsingle-plane 1��� */

	outportw(0x102, 0x0101);	/* graphic0 Width640 4dot/pixel */
								/* graphic1 Width640 4dot/pixel */

	outportb(0x153, 0x44);		/* G-VRAM�I�� */

	outportw(0x106, 0xab90);	/* �p���b�g�w���ʊ����Ďw�� */
	outportw(0x108, 0x0000);	/* ���ڐF�w���ʊ����Đݒ� */
	outportw(0x110, 0x008f);	/* 4�r�b�g�s�N�Z�� */
//	outportb(0x500, 0);	/* �Ɨ��A�N�Z�X */
//	outportb(0x512, 0);	/* �u���b�N0 */
//	outportb(0x516, 0);	/* �������݃v���[���I�� */
}

/*�I������*/
void end()
{
	outportw(0x100, 0xb000);	/* none-interless Graphic-on 400dot(400line) */
	outportb(0x153, 0x41);	/* T-VRAM�I�� */
	outportw(0x106, 0xab98);	/* �p���b�g�w���ʊ����Ďw�� */
}

/*�J�[�\���y�уt�@���N�V�����L�[�\���̐���*/
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

/*�e�L�X�g��ʋy�уO���t�B�b�N��ʂ̏���*/
void clear(short type)
{
	if(type & 1)
		paint(0x0);

//	if(type & 2)
//		printf("\x1b*");
}


int main(void)
{
	unsigned short i, j, k;
	unsigned short a, b;

	g_init();

	for(i = 0; i < 80; i++){
		star_y[i] = rand() % 50 * 8;
		star_inc[i] = rand() % 3 + 1;
		star_color[i] = rand() % 6 + 1;
	}

	clear(3);

//	do{

//	k = 100;
	while((inportb(0x09)) & 0x80){ /* ESC */
//	while(--k){
		for(i = 0; i < 80; i++){
			a = i + star_y[i] * 80;
			b = a + star_inc[i] * 80;
			star_y[i] += star_inc[i];
			star_y[i] %= 200;

			for(j = 0; j < 3; j++){
				if((star_color[i] & (1 << j)) != 0){
					*(flame[j] + a) &= (~PAT) ;
					*(flame[j] + b) |= PAT;
				}
			}
		}
		while((inportb(0x0040) & 0x20));
		while(!(inportb(0x0040) & 0x20)); /* WAIT VSYNC */
	}

//	clear(3);

	end();

	return 0;
}

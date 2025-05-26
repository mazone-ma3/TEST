/* MSX-SC5->PC-AT CONV.(Digital 8 colors) for GCC-ia16 By m@3 */
/* �S�̔� */

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>

#define ON 1
#define OFF 0
#define PAT 1

/************************************************************************/
/*		BIT����}�N����`												*/
/************************************************************************/

/* BIT�f�[�^�Z�o */
#define BITDATA(n) (1 << (n))

/* BIT�Z�b�g */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BIT�N���A */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BIT�`�F�b�N */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT���] */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}

#define WIDTH 32
#define LINE 212
#define RAM_ADR 0x2000


unsigned char __far *vram_adr = MK_FP(0xa000,0);

FILE *stream[2];

unsigned char conv_tbl[16] = { 0, 0, 4, 4, 1, 1, 2, 5, 2, 2, 6, 6 ,4 ,3 ,7 ,7 };

void cursor_switch(short);
void screen_switch(short);


int conv(char *loadfil)
{
	long i, j,count, count2;
	int k=0, l=0;
	unsigned char pattern[100];
	unsigned char pcatcolor[3];
	unsigned char msxcolor[8];
	unsigned char color;
	unsigned short header;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	fread(pattern, 1, 1, stream[0]);	/* MSX�擪��ǂݎ̂Ă� */
	fread(pattern, 1, 4, stream[0]);	/* MSX�w�b�_���ǂݎ̂Ă� */

	fread(pattern, 1, 2, stream[0]);	/* MSX�w�b�_��ǂݎ̂Ă� */

	for(count = 0; count < LINE; ++count){
		for(count2 = 0; count2 < WIDTH; ++count2){
	
			i = fread(pattern, 1, 2, stream[0]);	/* 4dot�� */
			if(i < 1)
				break;

			/* �F�����Ɗg�� */
			msxcolor[0] = (pattern[0] >>4) & 0x0f;
			msxcolor[1] = (pattern[0] >>4) & 0x0f;
			msxcolor[2] = pattern[0] & 0x0f;
			msxcolor[3] = pattern[0] & 0x0f;
			msxcolor[4] = (pattern[1] >>4) & 0x0f;
			msxcolor[5] = (pattern[1] >>4) & 0x0f;
			msxcolor[6] = pattern[1] & 0x0f;
			msxcolor[7] = pattern[1] & 0x0f;
			for(i = 0; i < 3; ++i){
				pcatcolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 3; ++i){
					color = conv_tbl[msxcolor[j]];	/* �F�ϊ� */
					if(BITTST(i, color)){
						BITSET(7-j, pcatcolor[i]);
					}else{
						BITCLR(7-j, pcatcolor[i]);
					}
				}
			}

			i = fread(pattern, 1, 2, stream[0]);	/* 4dot�� */
			if(i < 1)
				break;

			/* �F�����Ɗg�� */
			msxcolor[0] = (pattern[0] >>4) & 0x0f;
			msxcolor[1] = (pattern[0] >>4) & 0x0f;
			msxcolor[2] = pattern[0] & 0x0f;
			msxcolor[3] = pattern[0] & 0x0f;
			msxcolor[4] = (pattern[1] >>4) & 0x0f;
			msxcolor[5] = (pattern[1] >>4) & 0x0f;
			msxcolor[6] = pattern[1] & 0x0f;
			msxcolor[7] = pattern[1] & 0x0f;

			for(i = 0; i < 3; ++i){
				pattern[i] = pcatcolor[i];
			}

			for(i = 0; i < 3; ++i){
				pcatcolor[i] = 0;
			}

			for(j = 0; j < 8; ++j){
				for(i = 0; i < 3; ++i){
					color = conv_tbl[msxcolor[j]];	/* �F�ϊ� */
					if(BITTST(i, color)){
						BITSET(7-j, pcatcolor[i]);
					}else{
						BITCLR(7-j, pcatcolor[i]);
					}
				}
			}

			for(i = 0; i < 3; ++i){
				pattern[3 + i] = pcatcolor[i];
			}

			outportb(0x3ce, 0x05);	// write mode
			outportb(0x3cf, 0x00);
			outportb(0x3ce, 0x08);	// bit
			outportb(0x3cf, 0xff);

			outportb(0x3c4, 0x02);
			outportb(0x3c5, 0x01);
			*(vram_adr + 0 + k + l) = pattern[0];
			*(vram_adr + 1 + k + l) = pattern[3];
			*(vram_adr + 80 + k + l) = pattern[0];
			*(vram_adr + 81 + k + l) = pattern[3];

			outportb(0x3c5, 0x02);
			*(vram_adr + 0 + k + l) = pattern[1];
			*(vram_adr + 1 + k + l) = pattern[4];
			*(vram_adr + 80 + k + l) = pattern[1];
			*(vram_adr + 81 + k + l) = pattern[4];

			outportb(0x3c5, 0x04);
			*(vram_adr + 0 + k + l) = pattern[2];
			*(vram_adr + 1 + k + l) = pattern[5];
			*(vram_adr + 80 + k + l) = pattern[2];
			*(vram_adr + 81 + k + l) = pattern[5];

			outportb(0x3c5, 0x08);
			*(vram_adr + 0 + k + l) = 0;
			*(vram_adr + 1 + k + l) = 0;
			*(vram_adr + 80 + k + l) = 0;
			*(vram_adr + 81 + k + l) = 0;

			k += 2;
			if(k >= 64){
				k = 0;
				l += 80*2;
			}
		}
	}
	fclose(stream[0]);

	return 0;
}

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

/*�I������*/
void end()
{
	reg.h.ah = 0x00;
	reg.h.al = old_mode;
	int86(0x10, &reg, &reg);
}

void paint(unsigned char plane, unsigned char color)
{
	unsigned short i;

	outportb(0x3c4,0x02);
	outportb(0x3c5,plane);	/* �e�v���[���ւ̏������ݗL�� */

	for (i = 0; i < (80 * 480L); ++i)
		*(vram_adr + i) = color;
}

/*�e�L�X�g��ʋy�уO���t�B�b�N��ʂ̏���*/
void clear(short type)
{
	if(type & 1)
		paint(0x0f, 0);

	if(type & 2)
		printf("\x1b*");
}


/*�p���b�g�E�Z�b�g*/
void pal_set(unsigned char color, unsigned char red, unsigned char blue, unsigned char green)
{
/*	reg.h.ah = 0x10;
	reg.h.al = 0x10;
	reg.x.bx = color;
	reg.h.dh = red;
	reg.h.ch = blue;
	reg.h.cl = green;
	int86(0x10, &reg, &reg);
*/
	outportb(0x3c8, color);
	outportb(0x3c9, red);
	outportb(0x3c9, blue);
	outportb(0x3c9, green);}

void pal_all(unsigned char color[16][3])
{
	short i;
	for(i = 0; i < 16; i++)
		pal_set(i, ((color[i][0] + 1)*4-1) * (color[i][0] != 0), ((color[i][1]+1)*4-1) * (color[i][1] != 0), ((color[i][2]+1)*4-1) * (color[i][2] != 0));
}


int	main(int argc,char **argv){

	short i,j;
	unsigned short a, b;

	unsigned char pal[16][3] =
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

	if (argv[1] == NULL){
		printf("MSX .SC5 file Converter for PC-AT.\n");
		return 1;
	}
	g_init();
	for(i = 0; i < 16; ++i){
		reg.x.ax = 0x1000;
		reg.h.bl = i;
		reg.h.bh = i;

		int86(0x10, &reg, &reg);
	}
	pal_all(pal);

	clear(3);
	conv(argv[1]);

	getch();
	end();

	return 0;
}

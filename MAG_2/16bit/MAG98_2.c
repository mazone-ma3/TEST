/* PC-98 MAG to Screen CONV.(Analog 16 colors) for GCC-ia16 By m@3 */
/* �S�̔� */

#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

/* �L���[�o�b�t�@�p�ϐ� (�L���[�̓�����head�A�o����tail) */
//typedef struct{
	unsigned short	buffer_head;	/* �i�[(in)�ʒu�I�t�Z�b�g */
	unsigned short	buffer_tail;	/* ��o(out)�ʒu�I�t�Z�b�g */
	unsigned short	buffer_size;	/* �o�b�t�@�T�C�Y */
	short			error;	/* �G���[�ۑ� */
//	unsigned char	*data;	/* �o�b�t�@�ւ̃|�C���^ */
//}QUE_BUFFER;

/* �L���[�o�b�t�@���͏����}�N�� */
/* (���Z���g��Ȃ�) */
#define QUE_PUT(BUFF, DATA, ERRCODE) {	\
	buffer_top = buffer_head;			\
	if((++buffer_top) >= buffer_size)	\
		buffer_top = 0;					\
	if(buffer_top == buffer_tail){		\
		ERRCODE;						\
	}else{								\
	buffer_data[buffer_head] = DATA;	\
	buffer_head = buffer_top;			\
	}\
}

/* �L���[�o�b�t�@�o�͏����}�N�� */
#define QUE_GET(BUFF, DATA, ERRCODE) {	\
	if(buffer_tail == buffer_head){	\
		ERRCODE;						\
	}else{								\
	DATA = buffer_data[buffer_tail];	\
	if((++buffer_tail)>= buffer_size)	\
		buffer_tail = 0;				\
	}\
}

//QUE_BUFFER *buffer;
//QUE_BUFFER bufferdata;
unsigned short buffer_top, buffer_tmp;
unsigned char color = 0;

/************************************************************************/
/*		BIT����}�N����`												*/
/************************************************************************/

/* BIT�f�[�^�Z�o */
//#define BITDATA(n) (1 << (n))
unsigned char bitdata[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
#define BITDATA(b) bitdata[b]

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

unsigned char __far *flame[4]
	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};

FILE *stream[2];

void cursor_switch(short);
void screen_switch(short);

unsigned char start_header, machine_code, machine_flag,screen_mode;
unsigned short start_x, start_y, end_x, end_y;
unsigned long flagA_offset, flagB_offset, flagA_size, flagB_size, pixel_offset, pixel_size;

unsigned char *pallet;

unsigned short x = 0, y = 0;
unsigned char flagA_bit;

#define BUFFSIZE 8192 //16384 //42000L
#define PATTERN_SIZE 14000 //16384

unsigned char pattern[PATTERN_SIZE];
unsigned char *buffer_data;

void pset(unsigned short offset, unsigned char color[8])
{
	unsigned char i, j, *oldcolor;
	register unsigned char bit = 1;

	for(i = 0; i < 4; ++i){
		register unsigned char data;
		oldcolor = color;
		for(j = 0; j < 8; j++){
			data *= 2;
			if(*(oldcolor++) & bit){
				data |= 0x01;
			}
		}
		flame[i][offset] = data;
		bit *= 2;
	}
}

int conv(char *loadfil, unsigned char __far *flame[3])
{
	unsigned short i, j;
	unsigned short k=0, l=0;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.\n", loadfil);

		fclose(stream[0]);
		return ERROR;
	}
	fread(pattern, 1, 8, stream[0]);
	pattern[8] = '\0';
	if(strcmp(pattern, "MAKI02  ")){
		printf("not .MAG files %s.\n", loadfil);
		fclose(stream[0]);
		return ERROR;
	}
	fread(pattern, 1, 18, stream[0]);	/* User */
	pattern[18] = '\0';
	printf("User:%s\n",pattern);
	i = 0;
	printf("Memo:");
	do{
		fread(pattern, 1, 1, stream[0]);
		putchar(pattern[0]);
	}while(pattern[0] != '\x1a');
	putchar('\n');
	fread(pattern, 1, 32, stream[0]);

	start_header = pattern[0];
	printf("\n�w�b�_�擪 %d\n", start_header);
	machine_code = pattern[1];
	printf("�@��R�[�h %d\n", machine_code);
	machine_flag = pattern[2];
	printf("�@��ˑ��t���O %d\n", machine_flag);
	screen_mode = pattern[3];
	printf("�X�N���[�����[�h %d\n", screen_mode);
	start_x = pattern[4] + pattern[5] * 256;
	printf("�J�n�ʒuX %d\n", start_x);
	start_y = pattern[6] + pattern[7] * 256;
	printf("�J�n�ʒuY %d\n", start_y);
	end_x = pattern[8] + pattern[9] * 256;
	printf("�I���ʒuX %d\n", end_x);
	end_y = pattern[10] + pattern[11] * 256;
	printf("�I���ʒuY %d\n", end_y);
	flagA_offset = pattern[12] + pattern[13] * 256 + pattern[14] * 65536L + pattern[15] * 16777216L;
	printf("�t���OA�I�t�Z�b�g %d\n", flagA_offset);
	flagB_offset = pattern[16] + pattern[17] * 256 + pattern[18] * 65536L + pattern[19] * 16777216L;
	printf("�t���OB�I�t�Z�b�g %d\n", flagB_offset);
	flagB_size = pattern[20] + pattern[21] * 256 + pattern[22] * 65536L + pattern[23] * 16777216L;
	printf("�t���OB�T�C�Y %d\n", flagB_size);
	pixel_offset = pattern[24] + pattern[25] * 256 + pattern[26] * 65536L + pattern[27] * 16777216L;
	printf("�s�N�Z���̃I�t�Z�b�g %d\n", pixel_offset);
	pixel_size = pattern[28] + pattern[29] * 256 + pattern[30] * 65536L + pattern[31] * 16777216L;
	printf("�s�N�Z���̃T�C�Y %d\n", pixel_size);

	unsigned short width =  (end_x & 0xFFF8 | 7) - (start_x & 0xFFF8) + 1;
	unsigned short height = end_y - start_y;
	unsigned short colors = screen_mode & 0x80 ? 256 : 16;

	printf("Width = %d, Height = %d, colors = %d\n", width, height, colors);
	flagA_size = flagB_offset - flagA_offset;
	printf("�t���OA�T�C�Y %d\n", flagA_size);

	unsigned char pixelUnitLog = screen_mode & 0x80 ? 1 : 2;
	unsigned short flagSize = width >> (pixelUnitLog + 1);
	printf("�t���O�T�C�Y %d\n", flagSize);
//	unsigned short copysize = 4 << pixelUnitLog;
	unsigned short copysize = 1 << pixelUnitLog;
	printf("�R�s�[�T�C�Y %d\n", copysize);

	unsigned short pixel_max = pixel_size;

	if(pixel_offset> PATTERN_SIZE){
		printf("�������I�[�o�[\n");
		goto end;
	}
	if(((pixel_offset + pixel_size)) > PATTERN_SIZE){
		pixel_max = PATTERN_SIZE - pixel_offset;
//		printf("�������I�[�o�[\n");
//		goto end;
	}

	pallet = pattern;
	fread(pallet, 1, colors * 3, stream[0]);	/* Pallet */
//	for(i = 0, j = 0; i < colors; ++i, j+=3){
//		printf("color = %d[G%x R%x B%x] ",i,pallet[j], pallet[j+1], pallet[j+2]);
//	}

	unsigned char *flagABuf = pallet + colors * 3;
	unsigned char *flagBBuf = flagABuf + flagA_size;
	unsigned char *pixel = flagBBuf + flagB_size;

	fread(flagABuf, 1, flagA_size, stream[0]);
	fread(flagBBuf, 1, flagB_size, stream[0]);
	fread(pixel, 1, pixel_max, stream[0]);
	pixel_size -= pixel_max;

	unsigned char v,vv;
	unsigned short src = 0, dest = 0, old_dest = 0;

	unsigned short copypos[16];
	unsigned char copyx[16] = {0, 1, 2, 4, 0, 1, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
	unsigned char copyy[16] = {0, 0, 0, 0, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16};
	for (i = 0; i < 16; ++i) {
		copypos[i] = (copyy[i] * width + (copyx[i] << pixelUnitLog)); // * 4;
//		printf("%d/", copypos[i]);
	}


	unsigned char *flagBuff;
	flagBuff = (unsigned char *)malloc(flagSize);

	if(!flagBuff){
		printf("flagBuff Error.\n");
		goto end;
	}
	memset(flagBuff, 0, flagSize);

	flagA_bit = 0x80;
	unsigned short flagA_pos = 0;
	unsigned short flagB_pos = 0;
	unsigned short pointer = 0;
	j = 0;

	for( y = 0; y < height; ++y){
//		printf("\n y = %d ",y);
		// �t���O�f�[�^�W�J
		for(x =0; x< flagSize; ++x){
			if(flagABuf[flagA_pos] & flagA_bit){
				flagBuff[x] ^= flagBBuf[flagB_pos++];
			}
			if((flagA_bit /= 2) == 0){
				flagA_bit = 0x80;
				++flagA_pos;
			}
//		}
		// �s�N�Z���f�[�^�W�J
//		for(x = 0; x< flagSize; ++x){
			// �t���O1��4�r�b�g
			vv = flagBuff[x];

			for(k = 0; k < 2; ++k){
				if(k == 0)
					v = vv / 16;
				else
					v = vv % 16;

				if(!v){
					// 0�Ȃ�s�N�Z���f�[�^����1�s�N�Z��(2�o�C�g)�ǂ�
					if (colors == 16) {
//		printf("%d ",x);
						unsigned char err=0;
						for(i = 0;  i < 2; ++i){
							color = pixel[src++];

							if(src >= pixel_max){
								fread(pixel, 1, pixel_max, stream[0]);
								if((pixel_size < pixel_max)){
									pixel_max = pixel_size;
									pixel_size = 0;
								}else
									pixel_size -= pixel_max;
								src = 0;
							}
							QUE_PUT(bufferdata, color, err=1);
							if(err == 1)
								printf("ERROR\n");
						}
					} else {
					}
				}else{
//		printf("*%d ",x);
					// 0�ȊO�Ȃ�w��ʒu����1�s�N�Z��(16�F�Ȃ�4�h�b�g/256�F�Ȃ�2�h�b�g)�R�s�[
					short copySrc = (buffer_head - copypos[v] / 2), j = 0;
//					printf("(%d %d)",copySrc, copySrc % buffer_size);
					if(copySrc < 0){
//						printf("MINUS\n");
						copySrc += buffer_size;
					}
					if(copySrc > buffer_size){
//						printf("LARGE\n");
						copySrc %= buffer_size;
					}
					for(i = 0, j = copySrc; i < copysize / 2; ++i){
						unsigned char err=0;
						QUE_PUT(bufferdata, buffer_data[j], err=1);
						if((++j) >= buffer_size){
							j = 0;
						}
						if(err == 1)
							printf("ERROR\n");
					}
				}
			}
		}

		do{
			unsigned char color[8], data = 0, j = 0;
			unsigned char err=0;
			for(i = 0, j = 0; i < 4; i++){
				QUE_GET(bufferdata, data, err = 1);
				color[j++] = (data >>4) & 0x0f;
				color[j++] =  data & 0x0f;

				if(err == 1){
					printf("ERROR\n");
					goto end1;
				}
			}
			pset(pointer, color);
			pointer++;
		}while(buffer_head != buffer_tail);
	}
end1:
	free(flagBuff);
end:
	fclose(stream[0]);

	return NOERROR;
}

void g_init(void)
{
	_disable();
	outportb(0x6a, 1   ); /* 16�F���[�h (0x6a=mode f/fp2)*/
	outportb(0xa2, 0x4b);
	outportb(0xa0, 0); /* L/R = 1 (�c�����̊g��W��)*/
	outportb(0x68, 8   ); /* ���[�hF/F1 (8�ō��𑜓x)*/
	outportb(0x4a0,0xfff0);
	outportb(0x7c, 0);
	_enable();
}


/*�J�[�\���y�уt�@���N�V�����L�[�\���̐���*/
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
		outportb(0xa2, 0x0d); /* �\���J�n */
	else
		outportb(0xa2, 0x0c);
}


/*�y�[�W�؂�ւ�*/
void setpage(short visual, short active)
{
	outportb(0xa4, visual);
	outportb(0xa6, active);
}

/*�p���b�g�E�Z�b�g*/
void pal_set(short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	outportb(0xa8, color);
	outportb(0xaa, green);
	outportb(0xac, red);
	outportb(0xae, blue);
}

void pal_all(unsigned char color[16][3])
{
	short i;
	for(i = 0; i < 16; i++)
		pal_set(i, color[i][0], color[i][1], color[i][2]);
}


int	main(int argc,char **argv){
	char dst[100];

	if ((argv[1] == NULL) || (argc < 2)){
		printf(".MAG to Screen Converter for PC-98.\n");
		return ERROR;
	}

	screen_switch(OFF);
	g_init();
	setpage(0,0);
	screen_switch(ON);

	int len = strlen(argv[1]);
	if(len <= 5)
		snprintf(dst, sizeof dst, "%s.mag", argv[1]);
	else if((strcmp(&argv[1][len - 4] , ".mag")) && (strcmp(&argv[1][len - 4] , ".MAG"))){
		snprintf(dst, sizeof dst, "%s.mag", argv[1]);
	}else
		memcpy(dst,argv[1],len + 1);

	buffer_data = (unsigned char *)malloc(BUFFSIZE);
	if(!buffer_data){
		printf("grpdata buffer error.\n");
		return ERROR;
	}
	memset(buffer_data, 0, BUFFSIZE);

//	buffer = &bufferdata;
	buffer_size = BUFFSIZE;
//	buffer_data = grpdata;
	buffer_head = buffer_tail = 0;
	buffer_top = buffer_head;

	if(conv(dst, flame)){
		free(buffer_data);
		return ERROR;
	}
/*	snprintf(dst, sizeof dst, "%s.grr", argv[1]);
	conv(dst, flame[1]);
	snprintf(dst, sizeof dst, "%s.grg", argv[1]);
	conv(dst, flame[2]);
	snprintf(dst, sizeof dst, "%s.gri", argv[1]);
	conv(dst, flame[3]);
*/
	free(buffer_data);

	return NOERROR;
}

/* X1 MAG to Screen CONV.(Digital 8 colors) for zsdcc By m@3 */
/* 全体版 */

//#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define VRAM_TOP 0x4000

/* キューバッファ用変数 (キューの入口がhead、出口がtail) */
//typedef struct{
	unsigned short	buffer_head;	/* 格納(in)位置オフセット */
	unsigned short	buffer_tail;	/* 取出(out)位置オフセット */
	unsigned short	buffer_size;	/* バッファサイズ */
	short			error;	/* エラー保存 */
//	unsigned char	*data;	/* バッファへのポインタ */
//}QUE_BUFFER;

/* キューバッファ入力処理マクロ */
/* (除算を使わない) */
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

/* キューバッファ出力処理マクロ */
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

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
//#define BITDATA(n) (1 << (n))
unsigned char bitdata[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
#define BITDATA(n) bitdata[n]

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

//unsigned char __far *flame[4]
//	 = {MK_FP(0xa800,0)	,MK_FP(0xb000,0),MK_FP(0xb800,0),MK_FP(0xe000,0)};

FILE *stream[1];

unsigned char start_header, machine_code, machine_flag,screen_mode;
unsigned short start_x, start_y, end_x, end_y;
unsigned long flagA_offset, flagB_offset, flagA_size, flagB_size, pixel_offset, pixel_size;

unsigned char *pallet;

unsigned short x = 0, y = 0;
unsigned char flagA_bit;

#define BUFFSIZE 8192 //16384 //42000L
#define PATTERN_SIZE 14000 //20000 //14000

//unsigned char *pattern;
unsigned char pattern[PATTERN_SIZE];
unsigned char buffer_data[BUFFSIZE];

unsigned short offset_x = 0;
unsigned short offset_y = 0;
unsigned char locate = 0x80;

unsigned char color = 0;

unsigned short adr;

#define VRAM_MACRO(X, Y) (X / 8  + (Y / 8) * 80 + (Y & 7) * 0x800)

unsigned short y_table[200];

unsigned char newcolor[3];

void pset(unsigned short offset, unsigned char color[8]) __sdcccall(1)
{
/*	unsigned char i, j, *oldcolor;
	register unsigned char bit = 1;

	for(i = 0; i < 3; ++i){
		register unsigned char data;
		oldcolor = color;
		for(j = 0; j < 8; j++){
			data *= 2;
			if(*(oldcolor++) & bit){
				data |= 0x01;
			}
		}
		offset += 0x4000;
		outp(offset ,data);
		bit *= 2;
	}*/
__asm
	ex	de,hl
;;	ld	hl,2
;;	add	hl,sp
;	ld	e,(hl)
;	inc	hl
;	ld	d,(hl)	;de = offset
;	push	de
;	inc	hl
;;	ld	c,(hl)
;;	inc	hl
;;	ld	b,(hl)
;;	ld	l,c
;;	ld	h,b	;hl = color

	push	de

	ld	d,1
	ld	c,3
pset1:
	ld	b,8
	push	hl
pset2:
	ld	a,(hl)
	inc	hl
	and	d
	jr	z,pset3
	scf
pset3:
	rl	e
	djnz	pset2
	ld	hl,_newcolor-1
	add	hl,bc
	ld	(hl),e
	pop	hl
	sla	d
	dec	c
	jr	nz,pset1

	pop	hl	; hl = offset
	ld	de,#0x4000
;	add	hl,de
	ld	c,l
	ld	b,h
	ld	a,(_newcolor+2)
	out (c),a
	add	hl,de
	ld	c,l
	ld	b,h
	ld	a,(_newcolor+1)
	out (c),a
	add	hl,de
	ld	c,l
	ld	b,h
	ld	a,(_newcolor)
	out (c),a

__endasm;
}

unsigned char flagBuff[80]; //20000]; //16384];
unsigned short copypos[16];

unsigned char *flagABuf;
unsigned char *flagBBuf;
unsigned char *pixel;

unsigned char i;
unsigned char k=0;

unsigned char v,vv;
unsigned short src = 0;

unsigned char copyx[16] = {0, 1, 2, 4, 0, 1, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
unsigned char copyy[16] = {0, 0, 0, 0, 1, 1, 2, 2, 2, 4, 4, 4, 8, 8, 8, 16};

unsigned short flagA_pos;
unsigned short flagB_pos;

int conv(char *loadfil)
//, unsigned char __far *flame[3])
{

	offset_x = 0x4000;
	offset_y = 0;
	locate = 0x80;

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
//	printf("User:%s\n",pattern);
	i = 0;
//	printf("Memo:");
	do{
		fread(pattern, 1, 1, stream[0]);
//		putchar(pattern[0]);
	}while(pattern[0] != '\x1a');
//	putchar('\n');
	fread(pattern, 1, 32, stream[0]);

	start_header = pattern[0];
	printf("header_top %d\n", start_header);
	machine_code = pattern[1];
	printf("machine code %d\n", machine_code);
	machine_flag = pattern[2];
	printf("machine flag %d\n", machine_flag);
	screen_mode = pattern[3];
	printf("screen mode %d\n", screen_mode);
	start_x = pattern[4] + pattern[5] * 256;
	printf("start X %d\n", start_x);
	start_y = pattern[6] + pattern[7] * 256;
	printf("start Y %d\n", start_y);
	end_x = pattern[8] + pattern[9] * 256;
	printf("end X %d\n", end_x);
	end_y = pattern[10] + pattern[11] * 256;
	printf("end Y %d\n", end_y);
	flagA_offset = pattern[12] + pattern[13] * 256 + pattern[14] * 65536L + pattern[15] * 16777216L;
	printf("flagA offset %d\n", flagA_offset);
	flagB_offset = pattern[16] + pattern[17] * 256 + pattern[18] * 65536L + pattern[19] * 16777216L;
	printf("flagB offset %d\n", flagB_offset);
	flagB_size = pattern[20] + pattern[21] * 256 + pattern[22] * 65536L + pattern[23] * 16777216L;
	printf("flagB size %d\n", flagB_size);
	pixel_offset = pattern[24] + pattern[25] * 256 + pattern[26] * 65536L + pattern[27] * 16777216L;
	printf("pixel offset %d\n", pixel_offset);
	pixel_size = pattern[28] + pattern[29] * 256 + pattern[30] * 65536L + pattern[31] * 16777216L;
	printf("pixel size %d\n", pixel_size);

	unsigned short width =  (end_x & 0xFFF8 | 7) - (start_x & 0xFFF8) + 1;
	unsigned short height = end_y - start_y + 1;
	if(height >= 200)
		height = 200;
	unsigned short colors = screen_mode & 0x80 ? 256 : 16;

	printf("Width = %d, Height = %d, colors = %d\n", width, height, colors);
	flagA_size = flagB_offset - flagA_offset;
	printf("flagA size %d\n", flagA_size);

	unsigned char pixelUnitLog = screen_mode & 0x80 ? 1 : 2;
	unsigned short flagSize = width >> (pixelUnitLog + 1);
	printf("flag size %d\n", flagSize);
///	unsigned short copysize = 4 << pixelUnitLog;
	unsigned short copysize = 1 << pixelUnitLog;
	printf("copy size %d\n", copysize);

	unsigned long pixel_max = pixel_size;

	if(pixel_offset> PATTERN_SIZE){
//		printf("メモリオーバー\n");
		printf("memory over.\n");
		goto end;
	}
	if(((pixel_offset + pixel_size)) > PATTERN_SIZE){
		pixel_max = PATTERN_SIZE - pixel_offset;
//		printf("メモリオーバー\n");
//		goto end;
	}

	pallet = pattern;
	fread(pallet, 1, colors * 3, stream[0]);	/* Pallet */
//	for(i = 0, j = 0; i < colors; ++i, j+=3){
//		printf("color = %d[G%x R%x B%x] ",i,pallet[j], pallet[j+1], pallet[j+2]);
//	}

	flagABuf = pallet + colors * 3;
	flagBBuf = flagABuf + flagA_size;
	pixel = flagBBuf + flagB_size;

	fread(flagABuf, 1, flagA_size, stream[0]);
	fread(flagBBuf, 1, flagB_size, stream[0]);
	fread(pixel, 1, pixel_max, stream[0]);
	pixel_size -= pixel_max;

	for (i = 0; i < 16; ++i) {
		copypos[i] = (copyy[i] * width + (copyx[i] << pixelUnitLog)); // * 4;
//		printf("%d/", copypos[i]);
	}


//	unsigned char flagBuff[8192];
//	unsigned char *flagBuff;
//	flagBuff = (unsigned char *)malloc(flagSize);

//	if(!flagBuff)
//		goto end;
//	memset(flagBuff, 0, flagSize);

	flagA_bit = 0x80;
	flagA_pos = 0;
	flagB_pos = 0;

/*	for(i = 0; i < 8; ++i){
		bitdata[i] = 1 << i;
	}
*/
	adr = VRAM_TOP;
	for( y = 0; y < height; ++y){
//		printf("\n y = %d ",y);
		// フラグデータ展開
		for(x =0; x< flagSize; ++x){
/*			if(flagABuf[flagA_pos] & flagA_bit){
				flagBuff[x] ^= flagBBuf[flagB_pos++];
			}
			if((flagA_bit /= 2) == 0){
				flagA_bit = 0x80;
				++flagA_pos;
			}*/
//		}
__asm
	ld	bc,(_flagA_pos)
	ld	hl,(_flagABuf)
	add	hl,bc
	ld	a,(_flagA_bit)
	ld	b,(hl)
	and	a,b
	jr	z,skip1

	ld	bc,(_flagB_pos)
	ld	hl,(_flagBBuf)
	add	hl,bc
	inc	bc
	ld	(_flagB_pos),bc
	ld	d,(hl)

	ld	bc,(_x)
	ld	hl,_flagBuff
	add	hl,bc
	ld	a,(hl)
	xor	a,d
	ld	(hl),a

skip1:
	ld	a,(_flagA_bit)
	srl	a
	ld	(_flagA_bit),a
	jr	nz,skip2
	ld	a,0x80
	ld	(_flagA_bit),a
	ld	hl,(_flagA_pos)
	inc	hl
	ld	(_flagA_pos),hl
skip2:
__endasm;
		// ピクセルデータ展開
//		for(x = 0; x< flagSize; ++x){
			// フラグ1つ4ビット
			vv = flagBuff[x];

			for(k = 0; k < 2; ++k){
				if(k == 0)
					v = (vv >> 4) & 0x0f;
				else
					v = vv & 0x0f;

				if(!v){
					// 0ならピクセルデータから1ピクセル(2バイト)読む
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
//			printf("*%d ",x);
					// 0以外なら指定位置から1ピクセル(16色なら4ドット/256色なら2ドット)コピー
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
				color[j++] = (data >> 4) & 0x0f;
				color[j++] = data & 0x0f;
				if(err == 1){
					printf("ERROR\n");
					goto end;
				}
			}

			pset(adr, color);

			if(++offset_x >= (VRAM_TOP + 80)){
				offset_x = VRAM_TOP;
				++offset_y;
				adr = y_table[offset_y];
			}else
				adr++;

		}while(buffer_head != buffer_tail);
	}
//	free(flagBuff);
end:
	fclose(stream[0]);

	return NOERROR;
}

void setvramio(unsigned char color) __sdcccall(1)
{
__asm
;	ld	hl, 2
;	add	hl, sp
	ld	l,a

	ld	bc,#0x1a03
	ld	a,#0x0b
	out	(c),a
	ld	a,#0x0a
	out	(c),a

	ld	bc,0
;	ld	de,#0x3fff
loop:
;	ld	a, (hl)
	ld	a,l
	out	(c),a
	inc	bc
;	dec	de
;	ld	a,d
;	or	e
	ld	a,b
	cp	#0x40
	jr	nz,loop

__endasm;
}


/*パレット・セット*/
/*
void pal_set(short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
}

void pal_all(unsigned char color[16][3])
{
	short i;
	for(i = 0; i < 16; i++)
		pal_set(i, color[i][0], color[i][1], color[i][2]);
}
*/

int	main(int argc,char **argv){
	char dst[100];

	/* Pallet */
	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);

	if ((argv[1] == NULL) || (argc < 2)){
		printf(".MAG to Screen Converter for X1.\n");
		return ERROR;
	}


	int len = strlen(argv[1]);
	if(len <= 5)
		snprintf(dst, sizeof dst, "%s.mag", argv[1]);
	else if((strcmp(&argv[1][len - 4] , ".mag")) && (strcmp(&argv[1][len - 4] , ".MAG"))){
		snprintf(dst, sizeof dst, "%s.mag", argv[1]);
	}else
		memcpy(dst,argv[1],len + 1);

//	pattern = (unsigned char *)malloc(PATTERN_SIZE);
//	if(!pattern){
//		term();
//		return ERROR;
//	}
//	memset(pattern, 0, PATTERN_SIZE);

//	clear(3);
	setvramio(0);

//	for(i =0 ; i < 8; ++i)
//		printf("%d ", BITDATA(i));

	for(i = 0; i < 200; ++i)
		y_table[i] = VRAM_TOP + (i / 8) * 80 + (i & 7) * 0x800;

//	buffer = &bufferdata;
	buffer_size = BUFFSIZE;
//	buffer_data = grpdata;
	buffer_head = buffer_tail = 0;
	buffer_top = buffer_head;

	if(conv(dst))
		return ERROR;

//	for(i =0 ; i < 8; ++i)
//		printf("%d ", BITDATA(i));

/*	snprintf(dst, sizeof dst, "%s.grr", argv[1]);
	conv(dst, flame[1]);
	snprintf(dst, sizeof dst, "%s.grg", argv[1]);
	conv(dst, flame[2]);
	snprintf(dst, sizeof dst, "%s.gri", argv[1]);
	conv(dst, flame[3]);
*/
//	free(pattern);

	return NOERROR;
}

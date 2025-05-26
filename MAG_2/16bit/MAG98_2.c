/* PC-98 MAG to Screen CONV.(Analog 16 colors) for GCC-ia16 By m@3 */
/* 全体版 */

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
unsigned char color = 0;

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
//#define BITDATA(n) (1 << (n))
unsigned char bitdata[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
#define BITDATA(b) bitdata[b]

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
	printf("\nヘッダ先頭 %d\n", start_header);
	machine_code = pattern[1];
	printf("機種コード %d\n", machine_code);
	machine_flag = pattern[2];
	printf("機種依存フラグ %d\n", machine_flag);
	screen_mode = pattern[3];
	printf("スクリーンモード %d\n", screen_mode);
	start_x = pattern[4] + pattern[5] * 256;
	printf("開始位置X %d\n", start_x);
	start_y = pattern[6] + pattern[7] * 256;
	printf("開始位置Y %d\n", start_y);
	end_x = pattern[8] + pattern[9] * 256;
	printf("終了位置X %d\n", end_x);
	end_y = pattern[10] + pattern[11] * 256;
	printf("終了位置Y %d\n", end_y);
	flagA_offset = pattern[12] + pattern[13] * 256 + pattern[14] * 65536L + pattern[15] * 16777216L;
	printf("フラグAオフセット %d\n", flagA_offset);
	flagB_offset = pattern[16] + pattern[17] * 256 + pattern[18] * 65536L + pattern[19] * 16777216L;
	printf("フラグBオフセット %d\n", flagB_offset);
	flagB_size = pattern[20] + pattern[21] * 256 + pattern[22] * 65536L + pattern[23] * 16777216L;
	printf("フラグBサイズ %d\n", flagB_size);
	pixel_offset = pattern[24] + pattern[25] * 256 + pattern[26] * 65536L + pattern[27] * 16777216L;
	printf("ピクセルのオフセット %d\n", pixel_offset);
	pixel_size = pattern[28] + pattern[29] * 256 + pattern[30] * 65536L + pattern[31] * 16777216L;
	printf("ピクセルのサイズ %d\n", pixel_size);

	unsigned short width =  (end_x & 0xFFF8 | 7) - (start_x & 0xFFF8) + 1;
	unsigned short height = end_y - start_y;
	unsigned short colors = screen_mode & 0x80 ? 256 : 16;

	printf("Width = %d, Height = %d, colors = %d\n", width, height, colors);
	flagA_size = flagB_offset - flagA_offset;
	printf("フラグAサイズ %d\n", flagA_size);

	unsigned char pixelUnitLog = screen_mode & 0x80 ? 1 : 2;
	unsigned short flagSize = width >> (pixelUnitLog + 1);
	printf("フラグサイズ %d\n", flagSize);
//	unsigned short copysize = 4 << pixelUnitLog;
	unsigned short copysize = 1 << pixelUnitLog;
	printf("コピーサイズ %d\n", copysize);

	unsigned short pixel_max = pixel_size;

	if(pixel_offset> PATTERN_SIZE){
		printf("メモリオーバー\n");
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
		// フラグデータ展開
		for(x =0; x< flagSize; ++x){
			if(flagABuf[flagA_pos] & flagA_bit){
				flagBuff[x] ^= flagBBuf[flagB_pos++];
			}
			if((flagA_bit /= 2) == 0){
				flagA_bit = 0x80;
				++flagA_pos;
			}
//		}
		// ピクセルデータ展開
//		for(x = 0; x< flagSize; ++x){
			// フラグ1つ4ビット
			vv = flagBuff[x];

			for(k = 0; k < 2; ++k){
				if(k == 0)
					v = vv / 16;
				else
					v = vv % 16;

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
//		printf("*%d ",x);
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
	outportb(0x6a, 1   ); /* 16色モード (0x6a=mode f/fp2)*/
	outportb(0xa2, 0x4b);
	outportb(0xa0, 0); /* L/R = 1 (縦方向の拡大係数)*/
	outportb(0x68, 8   ); /* モードF/F1 (8で高解像度)*/
	outportb(0x4a0,0xfff0);
	outportb(0x7c, 0);
	_enable();
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

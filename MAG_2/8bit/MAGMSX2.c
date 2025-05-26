/* MSX2 MAG to Screen CONV.(Analog 16 colors) for zsdcc By m@3 */
/* 全体版 */

//#define _BORLANDC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <conio.h>

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

/* キューバッファ用変数 (キューの入口がhead、出口がtail) */
//	typedef struct{
	unsigned short	buffer_head;	/* 格納(in)位置オフセット */
	unsigned short	buffer_tail;	/* 取出(out)位置オフセット */
	unsigned short	buffer_size;	/* バッファサイズ */
	short			error;	/* エラー保存 */
//	unsigned char	*buffer_data;	/* バッファへのポインタ */
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
//QUE_BUFFER buffer; //data;
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

#define MAXCOLOR 16

unsigned char org_pal2[MAXCOLOR][3] =
	{{ 0, 0, 0 },
	{ 0, 0, 15 },
	{ 15, 0, 0 },
	{ 15, 0, 15 },
	{ 0, 15, 0 },
	{ 0, 15, 15 },
	{ 15, 15, 0 },
	{ 15, 15, 15 },
	{ 0, 0, 0 },
	{ 0, 0, 7 },
	{ 7, 0, 0 },
	{ 7, 0, 7 },
	{ 0, 7, 0 },
	{ 0, 7, 7 },
	{ 7, 7, 0 },
	{ 7, 7, 7 },};

unsigned char org_pal[MAXCOLOR][3] = {
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

FILE *stream[1];

void cursor_switch(short);
void screen_switch(short);

unsigned char start_header, machine_code, machine_flag,screen_mode;
unsigned short start_x, start_y, end_x, end_y;
unsigned long flagA_offset, flagB_offset, flagA_size, flagB_size, pixel_offset, pixel_size;

unsigned char *pallet;

unsigned short x = 0, y = 0;
unsigned char flagA_bit;

#define BUFFSIZE 8192 //16384 //42000L
#define PATTERN_SIZE 14000 //16384 //14000 //20000 //14000

unsigned char buffer_data[BUFFSIZE];
unsigned char pattern[PATTERN_SIZE];

unsigned short pointer = 0;

unsigned char color = 0;

enum {
	VDP_READDATA = 0,
	VDP_READSTATUS = 1
};

enum {
	VDP_WRITEDATA = 0,
	VDP_WRITECONTROL = 1,
	VDP_WRITEPAL = 2,
	VDP_WRITEINDEX = 3
};

#define VDP_readport(no) (VDP_readadr + no)
#define VDP_writeport(no) (VDP_writeadr + no)

unsigned char VDP_readadr;
unsigned char VDP_writeadr;

/* mainromの指定番地の値を得る */
unsigned char read_mainrom(unsigned short adr) __sdcccall(1)
{
__asm
;	ld	 hl, #2
;	add	hl, sp
;	push	ix

;	ld	e, (hl)
;	inc	hl
;	ld	d, (hl)	; de=adr
;	ld	h,d
;	ld	l,e	; hl=adr

	ld	a,(#0xfcc1)	; exptbl
	call	#0x000c	; RDSLT

;	ld	l,a
;	ld	h,#0

;	pop	ix
__endasm;
}


void write_VDP(unsigned char regno, unsigned char data) __sdcccall(1)
{
//	outp(VDP_writeport(VDP_WRITECONTROL), data);
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x80 | regno);
__asm
	ld	h,a
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	ld	a,l
	out	(c),a
	ld	a,h
	set 7,a
	out	(c),a
__endasm;
}

unsigned char highadr;
unsigned short lowadr;

void write_vram_adr(void) //unsigned char highadr, unsigned short lowadr)
{
//	highadr = (adr >> 16) & 0xff;// / 65536L;
//	lowadr = adr & 0xffff; //% 65536L;
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	ld	de,(_lowadr)
	out	(c),e
	ld	a,d
	and	a,0x3f
	set	6,a
	out	(c),a
__endasm;
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x40 | ((lowadr >> 8) & 0x3f));
}

void write_vram_data(unsigned char data) __sdcccall(1)
{
__asm
//	outp(VDP_writeport(VDP_WRITEDATA), data);
	ld	b,a
	ld	a,(_VDP_writeadr)
	ld	c,a
	out	(c),b
__endasm;
}

void read_vram_adr(void) //unsigned char highadr, unsigned short lowadr)
{
//	highadr = (adr >> 16) & 0xff;// / 65536L;
//	lowadr = adr & 0xffff; //% 65536L;
	write_VDP(14, (((highadr  << 2) & 0x04) | (lowadr >> 14) & 0x03));
__asm
	ld	a,(_VDP_writeadr)
	inc	a
	ld	c,a
	ld	de,(_lowadr)
	out	(c),e
	ld	a,d
	and	a,0x3f
	out	(c),a
__endasm;
//	outp(VDP_writeport(VDP_WRITECONTROL), (lowadr & 0xff));
//	outp(VDP_writeport(VDP_WRITECONTROL), 0x00 | ((lowadr >> 8) & 0x3f));
}

unsigned char read_vram_data(void) __sdcccall(1)
{
__asm
	ld	a,(_VDP_readadr)
	ld	c,a
	in	a,(c)
__endasm;
//	return inp(VDP_readport(VDP_READDATA));
}
//#define read_vram_data() inp(VDP_readport(VDP_READDATA))

void set_displaypage(int page)
{
__asm
	DI
__endasm;
	write_VDP(2, (page << 5) & 0x60 | 0x1f);
__asm
	EI
__endasm;
}

unsigned char read_VDPstatus(int no) __sdcccall(1)
{
	unsigned char data;
__asm
	DI
__endasm;
	write_VDP(15, no);
	data = inp(VDP_readport(VDP_READSTATUS));
	write_VDP(15, 0);
__asm
	EI
__endasm;
	return data;
}

/* screenのBIOS切り替え */
void set_screenmode(unsigned char mode) __sdcccall(1)
{
__asm
;	ld	 hl, 2
;	add	hl, sp
	ld	l,a

	push	ix

	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x005f	; CHGMOD(MAINROM)

;	ld	a, (hl)	; a = mode
	ld	a,l

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

void set_screencolor(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x0062	; CHGCLR(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}

/*
void cls(void)
{
__asm
	push	ix
	ld	a,(#0xfcc1)	; exptbl
	ld	d,a
	ld	e,0
	push	de
	pop	iy
	ld ix,#0x00c3	; CLS(MAINROM)

	call	#0x001c	; CALSLT
	pop	ix
__endasm;
}*/

/*パレット・セット*/
void pal_set(unsigned char pal_no, unsigned char color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	unsigned char port = VDP_writeport(VDP_WRITEPAL);
	write_VDP(16, color);
	outp(port, red * 16 | blue);
	outp(port, green);
}

void pal_all(unsigned char pal_no, unsigned char color[MAXCOLOR][3])
{
	unsigned short i;
	for(i = 0; i < MAXCOLOR; i++)
		pal_set(pal_no, i, color[i][0]/2, color[i][1]/2, color[i][2]/2);
}

void wait_vsync(void)
{
	while((read_VDPstatus(2) & 0x40));
	while(!(read_VDPstatus(2) & 0x40)); /* WAIT VSYNC */
}

/*void sys_wait(unsigned char wait)
{
	unsigned char i;
	for(i = 0; i < wait; ++i)
		wait_vsync();
}
*/

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

unsigned char *forclr = 0xf3e9;
unsigned char *bakclr = 0xf3ea;
unsigned char *bdrclr = 0xf3eb;
unsigned char *oldscr = 0xfcb0;

unsigned char forclr_old, bakclr_old, bdrclr_old;

unsigned short flagA_pos;
unsigned short flagB_pos;

int conv(char *loadfil)
{
	pointer = 0;
	src = 0;

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.\n", loadfil);

//		fclose(stream[0]);
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
	if(height >= 212)
		height = 212;
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
//		printf("memory over.\n");
//		printf("メモリオーバー\n");
//		goto end;
	}
	printf("pixel_max %d\n", pixel_max);

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

/*
	printf("Pixel Data: ");
	for(int i =0; i < pixel_max; ++i){
		printf("%d ", pixel[i]);
	}
	printf("\n");
*/
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
	width /= 2;

	getch();

	forclr_old = *forclr;
	bakclr_old = *bakclr;
	bdrclr_old = *bdrclr;

	*forclr = 15;
	*bakclr = 0;
	*bdrclr = 0;
	set_screencolor();

	set_screenmode(7);
	pal_all(0, org_pal2);

__asm
	DI
__endasm;

	highadr = 0;
	lowadr = 0;
	write_vram_adr();
	for( y = 0; y < height; ++y){
//		printf("\n y = %d ",y);
//		printf("flagSize=%d",flagSize);
		// フラグデータ展開
		for(x =0; x< flagSize; ++x){
/*			if(flagABuf[flagA_pos] & flagA_bit){
				flagBuff[x] ^= flagBBuf[flagB_pos++];
			}
			if((flagA_bit /= 2) == 0){
				flagA_bit = 0x80;
				++flagA_pos;
			}*/
//		}*/
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
////		printf("Pixel");
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
//								goto end;
								fread(pixel, 1, pixel_max, stream[0]);
								if((pixel_size < pixel_max)){
									pixel_max = pixel_size;
									pixel_size = 0;
								}else
									pixel_size -= pixel_max;
								src = 0;
							}

							QUE_PUT(buffer, color, err=1);
							if(err == 1)
//								printf("ERROR\n");
								goto end;
						}
					} else {
					}
				}else{
//			printf("*%d ",x);
					// 0以外なら指定位置から1ピクセル(16色なら4ドット/256色なら2ドット)コピー
					short copySrc = (buffer_head - (copypos[v] / 2)), i, j = 0;
//					printf("(%d %d)",copySrc, copySrc % buffer_size);
					if(copySrc < 0){
//						printf("MINUS\n");
						copySrc += buffer_size;
					}
					if(copySrc > buffer_size){
//						printf("LARGE\n");
						copySrc %= buffer_size;
					}
					for(i = 0, j = copySrc; i < (copysize / 2); ++i){
						unsigned char err=0;
						QUE_PUT(buffer, buffer_data[j], err=1);
//						printf("(%d)",j);
						if((++j) >= buffer_size){
							j = 0;
						}
						if(err == 1)
//							printf("ERROR\n");
							goto end;
					}
				}

			}
		}

		do{
			unsigned char err=0;

			QUE_GET(buffer, color, err = 1);
			if(err == 1){
//				printf("ERROR\n");
				goto end;
			}

			if(pointer++ < (512 / 2))
				write_vram_data(color);
			else if(pointer == width)
				pointer = 0;
		}while(buffer_head != buffer_tail);
	}
	getch();
	goto end;

end:
__asm
	EI
__endasm;

	*forclr = forclr_old;
	*bakclr = bakclr_old;
	*bdrclr = bdrclr_old;
	set_screencolor();

	pal_all(0, org_pal);
	set_screenmode(*oldscr);
	fclose(stream[0]);

	return  NOERROR;
}

int	main(int argc,char **argv){
	char dst[100];

	VDP_readadr = read_mainrom(0x0006);
	VDP_writeadr = read_mainrom(0x0007);

	if ((argv[1] == NULL) || (argc < 2)){
		printf(".MAG to Screen Converter for MSX2.\n");
		return ERROR;
	}

	int len = strlen(argv[1]);
	if(len <= 5)
		snprintf(dst, sizeof dst, "%s.mag", argv[1]);
	else if((strcmp(&argv[1][len - 4] , ".mag")) && (strcmp(&argv[1][len - 4] , ".MAG"))){
		snprintf(dst, sizeof dst, "%s.mag", argv[1]);
	}else
		memcpy(dst,argv[1],len + 1);

//	buffer = &bufferdata;
	buffer_size = BUFFSIZE;
//	buffer_data = grpdata;
	buffer_head = buffer_tail = 0;
	buffer_top = buffer_head;

	if(conv(dst))
		return ERROR;

	return NOERROR;
}

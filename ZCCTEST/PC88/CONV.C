/* MSX-BIN->PC-88-BIN CONV. for GCC */
/* zccb88.bat‚Æ‘g‚İ‡‚í‚¹‚é(pc88_crt0.asm‚Ì’l‚ğ$8A00->$A700‚É•ÏX) */
#include <stdio.h>

FILE *stream[2];


int conv(char *loadfil, char*savefil)
{
	long i;
	unsigned char pattern[5];

	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", loadfil);

		fclose(stream[0]);
		return 1;
	}
	if ((stream[1] = fopen( savefil, "wb")) == NULL) {
		fprintf(stderr, "Can\'t open file %s.", savefil);

		fclose(stream[1]);
		return 1;
	}

	fread(pattern, 1, 1, stream[0]);
	fread(pattern, 1, 4, stream[0]);
	fwrite(pattern, 1, 4, stream[1]);
	fread(pattern, 1, 2, stream[0]);

	for(;;){
		i = fread(pattern, 1, 1, stream[0]);
		if(i < 1)
			break;
		i = fwrite(pattern, 1, 1, stream[1]);
		if(i < 1)
			break;
	}
	fclose(stream[0]);
	fclose(stream[1]);

	return 0;
}


int	main(int argc,char **argv){

	if (argv[1] == NULL)
		return 1;
	if (argv[2] == NULL)
		return 1;

	conv(argv[1], argv[2]);

	return 0;
}

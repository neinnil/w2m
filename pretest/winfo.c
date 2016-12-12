#include <stdio.h>
#include <stdlib.h>
#include "nein/wave.h"

int main (int ac, char** av)
{
	WAVE_FILE_INFO_T * info = NULL;

	if (NULL!=(info = getWaveInfoFromFile(av[1]))) {
		long length = 0;
		FILE *fp = fopen (av[1], "r");
		fseek(fp, 0L, SEEK_END);
		length = ftell(fp);
		fclose (fp);
		printf("\nFile: %s\n", av[1]);
		printf("length: %ld\n", length);
		printWaveInfo(info);
		FREEWAVEFILEINFO(info);
	}

	return 0;
}

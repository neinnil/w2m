#include <stdio.h>
#include <stdlib.h>
#include "nein_wave.h"

int main (int ac, char** av)
{
	WAVE_FILE_INFO_T * info = NULL;

	if (NULL!=(info = getWaveInfoFromFile(av[1]))) {
		printWaveInfo(info);
		FREEWAVEFILEINFO(info);
	}

	return 0;
}

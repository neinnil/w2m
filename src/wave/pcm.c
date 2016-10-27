#include <stdlib.h>
#include "nein/wave.h"
#include "nein/pcm.h"

/*
   The arugument of initPCM_Reader, pamhandle, must be a null pointer.
   It is not allocated previously.
   initPCM_Reader의 인자 pcmhandle은 NULL Pointer여야 한다.
   return value: if success returns zero. others returns -errors.
 */
int
initPCM_Reader(pcm_reader_data **pcmhandle)
{
	pcm_reader_data *p = NULL;
	if (!pcmhandle || *pcmhandle){
		return -EINVAL;
	}

	if(!(p = (pcm_reader_data*)malloc(sizeof(pcm_reader_data)))){
		return -ENOMEM;
	}

	memset (p, 0x00, sizeof(pcm_reader_data));

	*pcmhandle = p;
	return 0;
}

/**
  Open file stream, after to check this file is a supported wave file or not,
  if this file is supported then set file stream pointer to a pointer of pcm_reader_data. if not, this function close this file stream pointer and returns -EIVAL 
  */
int
setPCM_OpenWaveFile(pcm_reader_data *pcmhandle, const char *file)
{
	FILE *fp = NULL;
	WAVEINFO_BASE_T	*wvp = NULL;
	if (!pcmhandle) {
		return -EINVAL;
	}

	if(!(fp = fopen (file, "rb"))) {
		return -ENOENT;
	}

	if (!(wvp = getWaveInfo (fp))) {
		fclose (fp);
		return -EINVAL; /* to-do: speicify error code */
	}

	if (isSupportedWAVEFile(wvp)) {
		free (wvp);
		fclose (fp);
		return -EINVAL; /* to do: specifiy error code */
	}

	pcmhandle->info = (void*)wvp;
	pcmhandle->wave_in = fp;
	return 0;
}

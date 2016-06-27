#include <stdio.h>

#include "nein_error.h"
#include "nein_wave.h"

static
WAVEINFO_BASE_T*
allocWAVE_BASE(void)
{
	WAVEINFO_BASE_T *p = NULL;
	if(NULL==(p = (WAVEINFO_BASE_T*)malloc(sizeof(WAVEINFO_BASE_T)))){
		return NULL;
	}
	return p;
}

WAVEINFO_BASE_T*
getWaveInfoFromFile (const char* file)
{
	FILE *ifp = NULL;
	WAVEINFO_BASE_T *pOut = NULL;
	int  nread = 0;

	if (!file){
		return NULL;
	}
	if (!(ifp=fopen(file, "rb"))){
		error_print("File(%s) cannot be opened.\n", file);
		return NULL;
	}

	pOut = getWaveInfo (ifp);

	fclose (ifp);

	return pOut;
}

WAVEINFO_BASE_T*
getWaveInfo (FILE* infp)
{
	WAVEINFO_BASE_T *pOut = NULL;
	int  nread = 0;

	if (!infp){
		return NULL;
	}
	if (!(pOut=allocWAVE_BASE())){
		error_print("Fail to allocate WAVEINFO_BASE_T.\n");
		fclose (ifp);
		return NULL;
	}

	nread = fread(pOut, sizeof(WAVEINFO_BASE_T), 1, ifp);
	if (nread!=1){
		error_print("Fail to read from file\n");
		free(pOut);
		pOut = NULL;
	}

	return pOut;
}

int
isSupportedWAVEFile (WAVEINFO_BASE_T* waveinfo)
{
	if (!waveinfo) {
		return -EINVAL;
	}
	/* WAVE_FORMAT_PCM || WAVE_FORMAT_IEEE_FLOAT */
	if ((waveinfo->fmtTag == WAVE_FORMAT_PCM 
			|| waveinfo->fmtTag == WAVE_FORMAT_IEEE_FLOAT)
		&& waveinfo->fmt.chk_size == 16)
	{
		return 0;
	}
	return -NEIN_NOT_SUPPORTED;
}

short
getWAVEFormatTag (WAVEINFO_BASE_T* waveinfo)
{
	if (!waveinfo){
		return -EINVAL;
	}
	return waveinfo->fmtTag;
}

short
getWAVEChannels (WAVEINFO_BASE_T* waveinfo)
{
	if (!waveinfo){
		return -EINVAL;
	}
	return waveinfo->nChannels;
}

short
getWAVESampleRate (WAVEINFO_BASE_T* waveinfo)
{
	if (!waveinfo){
		return -EINVAL;
	}
	return waveinfo->nSamplePerSec;
}

short
getWAVEBitsPerSample (WAVEINFO_BASE_T* waveinfo)
{
	if (!waveinfo){
		return -EINVAL;
	}
	return waveinfo->BitsPerSample;
}

int
getWAVEDataLength (DATA_CHUNK_T *data)
{
	if (!data){
		return -EINVAL;
	}
	return data->datalen;
}

int
convtWAVEBASE2PCM(WAVEINFO_BASE_T** waveinfo)
{
	if (!waveinfo || !*waveinfo){
		return -EINVAL;
	}
	if (NULL = 
		(*waveinfo = (WAVE_PCM_T*)realloc((void*)*waveinfo, sizeof(WAVE_PCM_T))))
	{
		return -errno;
	}
	return 0;
}

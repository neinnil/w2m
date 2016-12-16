#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include "nein/wave.h"
#include "nein/pcm.h"


static void freePcmBuffer (PcmBuffer *pbuf);

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
	int	  rc = -EINVAL;;

	if (pcmhandle) {
		if(!(fp = fopen (file, "rb"))) {
			return -ENOENT;
		}
		rc = setPCM_WaveFile(pcmhandle, fp);
		if (rc != 0){
			fclose (fp);
		}
	}

	return rc;
}

void
freePCM_Reader(pcm_reader_data *pcmhandle)
{
	if (pcmhandle)
	{
		if (pcmhandle->wave_in) fclose (pcmhandle->wave_in);
		if (pcmhandle->in_id3v2_tag) free(pcmhandle->in_id3v2_tag);
		if (pcmhandle->info) {
			freeWaveInfo ((WAVE_FILE_INFO_T*)pcmhandle->info);
		}
		free (pcmhandle);
	}
}

int
setPCM_WaveFile(pcm_reader_data *pcmhandle, FILE *infp)
{
	if (pcmhandle && infp)
	{
		WAVE_FILE_INFO_T	*wfinfo = NULL;

		if (!(wfinfo = getWaveInfo (infp))) {
			return -EINVAL; /* to-do: speicify error code */
		}

		if (isSupportedWAVEFile(wfinfo)) {
			freeWaveInfo (wfinfo);
			return -EINVAL; /* to do: specifiy error code */
		}

		pcmhandle->info = (void*)wfinfo;
		pcmhandle->wave_in = infp;
		return 0;
	}
	return -EINVAL;
}

int
setPCM_FromWaveInfo(pcm_reader_data* pcmhandle, FILE *infp, void* waveinfo)
{
	if (pcmhandle)
	{
		if (waveinfo && infp)
		{
			pcmhandle->info = (void*)waveifno;
			pcmhandle->wave_in = infp;
		}
		else if (waveinfo)
		{
			pcmhandle->info = (void*)waveifno;
		}
		return 0;
	}
	return -EINVAL;
}

int
readPCM_data(pcm_reader_data* pcmhandle, int toread)
{
}

int
readPCM_data_int(pcm_reader_data* pcmhandle, int toread)
{
}

int
readPCM_data_short(pcm_reader_data* pcmhandle, int toread)
{
}

int
readPCM_data_ieee_float(pcm_reader_data* pcmhandle, int toread)
{
}


void
freePcmBuffer (PcmBuffer *pbuf)
{
	if (pbuf)
	{
		if (pbuf->ch[0]) free (pbuf->ch[0]);
		if (pbuf->ch[1]) free (pbuf->ch[1]);
	}
}
int
setPCM_file_position(pcm_reader_data* pcmhandle)
{
	int rc = -EINVAL;

	if (pcmhandle)
	{
		WAVE_FILE_INFO_T *wfinfo = NULL;
		winfo = (WAVE_FILE_INFO_T*)pcmhandle->info;
		if (winfo)
		{
			if (!pcmhandle->wave_in)
			{
				if (winfo->name)
				{
					pcmhandle->wave_in = fopen (winfo->name,"rb");
				}
			}
			if (pcmhandle->wave_in)
			{
				rc = fseek(pcmhandle->wave_in, winfo->datainfo.offset,SEEK_SET);
			}
		}
	}

	return rc;
}

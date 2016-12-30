#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "nein/wave.h"
#include "nein/pcm.h"
#include "nein/debug.h"

#define DEF_READ_NSAMPLE	1152
#define DEF_READ_SIZE		(2*DEF_READ_NSAMPLE)

static int	_setPCM_info_from_wavefileinfo(pcm_reader_data *pcmhdl);

/*
   The arugument of initPCM_Reader, pamhandle, must be a null pointer.
   It is not allocated previously.
   initPCM_Reader의 인자 pcmhdl은 NULL Pointer여야 한다.
   return value: if success returns zero. others returns -errors.
 */
int
initPCM_Reader(pcm_reader_data **pcmhdl, int useExtBuffer)
{
	pcm_reader_data *p = NULL;
	int rc =0;

	if (!pcmhdl || *pcmhdl)
	{
		return -EINVAL;
	}

	if(!(p = (pcm_reader_data*)malloc(sizeof(pcm_reader_data))))
	{
		return -ENOMEM;
	}

	memset (p, 0x00, sizeof(pcm_reader_data));
	if (!(useExtBuffer & EXT_PCM_BUFFER))
	{
		p->pcm16 = (PcmBuffer *)malloc (sizeof (PcmBuffer));
		rc = initPcmBuffer (p->pcm16, DEF_READ_NSAMPLE, (int)sizeof(int16_t));
		p->pcm32 = (PcmBuffer *)malloc (sizeof (PcmBuffer));
		rc |=initPcmBuffer (p->pcm32, DEF_READ_NSAMPLE, (int)sizeof(int32_t));
		p->pcmfloat = (PcmBuffer *)malloc (sizeof (PcmBuffer));
		rc |= initPcmBuffer (p->pcmfloat, DEF_READ_NSAMPLE, (int)sizeof(float));
		p->pcmdouble = (PcmBuffer *)malloc (sizeof (PcmBuffer));
		rc |= initPcmBuffer (p->pcmdouble, DEF_READ_NSAMPLE, (int)sizeof(double));

		if (rc == -ENOMEM || rc == -EINVAL)
		{
			freePcmBuffer (p->pcm16);
			freePcmBuffer (p->pcm32);
			freePcmBuffer (p->pcmfloat);
			freePcmBuffer (p->pcmdouble);
			if (p->pcm16)
			{
				free (p->pcm16);
			}
			if (p->pcm32)
			{
				free (p->pcm32);
			}
			if (p->pcmfloat)
			{
				free (p->pcmfloat);
			}
			if (p->pcmdouble)
			{
				free (p->pcmdouble);
			}
			return rc;
		}
	}
	p->useExtBuffer = useExtBuffer;

	*pcmhdl = p;

	return 0;
}

/**
  Open file stream, after to check this file is a supported wave file or not.
  If this file is supported then set file stream pointer to a pointer of pcm_reader_data. 
  If not, this function close this file stream pointer and returns -EIVAL 
  */
int
setPCM_OpenWaveFile(pcm_reader_data *pcmhdl, const char *file)
{
	FILE *fp = NULL;
	int	  rc = -EINVAL;;

	if (pcmhdl) {
		if(!(fp = fopen (file, "rb"))) {
			return -ENOENT;
		}
		rc = setPCM_WaveFile(pcmhdl, fp);
		if (rc != 0){
			fclose (fp);
		}
	}

	return rc;
}

void
freePCM_Reader(pcm_reader_data *pcmhdl)
{
	if (pcmhdl)
	{
		if (pcmhdl->wave_in) fclose (pcmhdl->wave_in);
		if (pcmhdl->in_id3v2_tag) free(pcmhdl->in_id3v2_tag);
		if (pcmhdl->info) {
			freeWaveInfo ((WAVE_FILE_INFO_T*)pcmhdl->info);
		}
		if (!(pcmhdl->useExtBuffer & EXT_PCM_BUFFER))
		{
			freePcmBuffer (pcmhdl->pcm16);
			freePcmBuffer (pcmhdl->pcm32);
			freePcmBuffer (pcmhdl->pcmfloat);
			freePcmBuffer (pcmhdl->pcmdouble);
			if (pcmhdl->pcm16)
			{
				free (pcmhdl->pcm16);
			}
			if (pcmhdl->pcm32)
			{
				free (pcmhdl->pcm32);
			}
			if (pcmhdl->pcmfloat)
			{
				free (pcmhdl->pcmfloat);
			}
			if (pcmhdl->pcmdouble)
			{
				free (pcmhdl->pcmdouble);
			}
		}
		free (pcmhdl);
	}
}

int
setPCM_WaveFile(pcm_reader_data *pcmhdl, FILE *infp)
{
	if (pcmhdl && infp)
	{
		WAVE_FILE_INFO_T	*wfinfo = NULL;

		if (!(wfinfo = getWaveInfo (infp))) {
			return -EINVAL; /* to-do: speicify error code */
		}

		if (isSupportedWAVEFile(wfinfo)) {
			freeWaveInfo (wfinfo);
			return -EINVAL; /* to do: specifiy error code */
		}

		pcmhdl->info = (void*)wfinfo;
		pcmhdl->wave_in = infp;
		_setPCM_info_from_wavefileinfo(pcmhdl);
		return 0;
	}
	return -EINVAL;
}

int
setPCM_FromWaveInfo(pcm_reader_data* pcmhdl, FILE *infp, void* waveinfo)
{
	if (pcmhdl)
	{
		if (waveinfo && infp)
		{
			pcmhdl->info = (void*)waveinfo;
			pcmhdl->wave_in = infp;
		}
		else if (waveinfo)
		{
			pcmhdl->info = (void*)waveinfo;
			if (!infp)
			{
				WAVE_FILE_INFO_T	*twfi = (WAVE_FILE_INFO_T*)waveinfo;
				pcmhdl->wave_in = fopen (twfi->name, "rb");
			}
		}
		else if (infp)
		{
			return setPCM_WaveFile (pcmhdl, infp);
		}
		else 
		{
			return -EINVAL;
		}
		_setPCM_info_from_wavefileinfo(pcmhdl);
		return 0;
	}
	return -EINVAL;
}

int
readPCM_data(pcm_reader_data* pcmhdl, int toread)
{

	if (pcmhdl && pcmhdl->wave_in)
	{
		if (pcmhdl->bitspersample == 64)
		{
			if (pcmhdl->is_ieee_float)
			{
				return readPCM_data_ieee_double(pcmhdl, toread);
			}
		}
		else if (pcmhdl->bitspersample > 16) 
		{
			if (pcmhdl->is_ieee_float)
			{
				return readPCM_data_ieee_float(pcmhdl, toread);
			}
			return readPCM_data_int(pcmhdl, toread);
		}
		else if (pcmhdl->bitspersample == 8)
		{
			return readPCM_data_uint8 (pcmhdl, toread);
		}
		else
		{
			return readPCM_data_short (pcmhdl, toread);
		}
	}
	return -1;
}

int
readPCM_data_uint8(pcm_reader_data* pcmhdl, int toread)
{
	if (pcmhdl && pcmhdl->wave_in)
	{
		unsigned char	buffer[DEF_READ_SIZE];
		int		nread = 0;
		int		samples_to_read = toread>DEF_READ_NSAMPLE?DEF_READ_NSAMPLE:toread;
		int		i;

		samples_to_read *= pcmhdl->nChannels;

		nread = fread (buffer, 1, samples_to_read, pcmhdl->wave_in);
		nread /= pcmhdl->nChannels;
		if (nread > 0)
		{
			short *pch[2] ;
			pch[0] = (short*)(pcmhdl->pcm16->ch[0]);
			pch[1] = (short*)(pcmhdl->pcm16->ch[1]);
			if (pcmhdl->nChannels == 2)
			{
				for (i=0; i<nread; i++)
				{
					pch[0][i] = ((short)(buffer[2*i] - 0x80) << 8 );
					pch[1][i] = ((short)(buffer[2*i+1] - 0x80) << 8 );
				}
			}
			else if (pcmhdl->nChannels == 1)
			{
				for (i=0; i<nread; i++)
				{
					pch[0][i] = ((short)(buffer[i] - 0x80) << 8 );
				}
			}
			else nread = -1;
		}
		return nread;
	}
	return -1;
}
#pragma pack(1)
typedef struct {
	unsigned char _v[3];
} int24_t;
#pragma pack(0)

int
readPCM_data_int(pcm_reader_data* pcmhdl, int toread)
{
	unsigned char buffer[DEF_READ_SIZE * sizeof(int)];
	int	*pch[2];
	int	i;
	int bytes_per_sample = 4;

	int nread = (DEF_READ_NSAMPLE>toread)?toread:DEF_READ_NSAMPLE;

	if (!pcmhdl || pcmhdl->nChannels < 1 || pcmhdl->nChannels > 2)
		return -1;
	nread *= pcmhdl->nChannels;
	bytes_per_sample = (pcmhdl->bitspersample + 7)/8;
	//nread *= pcmhdl->blockAlign;
	nread = fread (buffer, bytes_per_sample, nread, pcmhdl->wave_in);

	if (nread <= 0)
		return nread;

	pch[0] = (int*)(pcmhdl->pcm32->ch[0]);
	pch[1] = (int*)(pcmhdl->pcm32->ch[1]);

	nread /= pcmhdl->nChannels;
	if (24 == pcmhdl->bitspersample)
	{
		int24_t	*tmp = (int24_t*)buffer + (nread * pcmhdl->nChannels);
		if (2 == pcmhdl->nChannels)
		{
			for (i=nread; --i >=0 ; )
			{
				pch[1][i] = (*(int*)(--tmp))<<8;
				pch[0][i] = (*(int*)(--tmp))<<8;
			}
			/***
			for (i=0; i<nread; i++)
			{
				pch[0][i] = (*(int*)(tmp+2*i)) << 8;
				pch[1][i] = (*(int*)(tmp+2*i+1)) << 8;
			}
			***/
		}
		else if (1 == pcmhdl->nChannels)
		{
			for (i=nread; --i >=0 ; )
			{
				pch[0][i] = (*(int*)(--tmp))<<8;
			}
			/***
			for (i=0; i<nread; i++)
			{
				pch[0][i] = (*(int*)(tmp+i)) << 8;
			}
			****/
		}
	}
	else if (32 == pcmhdl->bitspersample)
	{
		int *pbuf = (int*)buffer + (nread * pcmhdl->nChannels);

		if (2 == pcmhdl->nChannels)
		{
			for (i=nread; --i >=0 ; )
			{
				pch[1][i] = *--pbuf;
				pch[0][i] = *--pbuf;
			}
		}
		else if (1 == pcmhdl->nChannels)
		{
			memcpy (pch[0], buffer, nread * sizeof(int));
		}
	}
	return nread;
}

int
readPCM_data_short(pcm_reader_data* pcmhdl, int toread)
{
	/* assume 16bits data. */
	int16_t buffer[DEF_READ_SIZE];
	int16_t *pbf_0 = (int16_t*)(pcmhdl->pcm16->ch[0]);
	int16_t *pbf_1 = (int16_t*)(pcmhdl->pcm16->ch[1]);
	int samples_to_read = (DEF_READ_NSAMPLE)>toread?toread:(DEF_READ_NSAMPLE);
	samples_to_read *= pcmhdl->nChannels;
	int nread = 0;

	nread = fread (buffer, sizeof(int16_t), samples_to_read, pcmhdl->wave_in);
	nread /= pcmhdl->nChannels;

	if (nread>0)
	{
		int i;
		if (pcmhdl->nChannels == 2)
		{
			for ( i=0; i<nread ; i++)
			{
				pbf_0[i] = buffer[i*2];
				pbf_1[i] = buffer[i*2+1];
			}
		} 
		else if (pcmhdl->nChannels == 1)
		{
			memcpy(pbf_0, buffer, nread*sizeof(short));
		}
		else 
		{
			return -1;
		}
#if 0
		if (pcmhdl->validBitsMask)
		{
			int b = sizeof(short)*8;
			for (i=0; i<nread; i++)
			{
				pbf_0[i] = (pbf_0[i] & pcmhdl->validBitsMask) << (b - pcmhdl->validBitsPerSample);
				if (pcmhdl->nChannels == 2)
				{
					pbf_1[i] = (pbf_1[i] & pcmhdl->validBitsMask) <<(b- pcmhdl->validBitsPerSample);
				}
			}
		}
#endif
	}

	return nread;
}

int
readPCM_data_ieee_float(pcm_reader_data* pcmhdl, int toread)
{
	int nread = -1;
	if (pcmhdl && pcmhdl->wave_in)
	{
		if (pcmhdl->bitspersample != sizeof(float)*8)
		{
			return -1;
		}
		float buffer[DEF_READ_SIZE];
		nread = (DEF_READ_NSAMPLE>toread)?toread:DEF_READ_NSAMPLE;
		nread *= pcmhdl->nChannels;
		nread = fread (buffer, sizeof(float), nread, pcmhdl->wave_in);
		
		if (nread > 0)
		{
			float *pch[2] ;
			pch[0] = (float*)(pcmhdl->pcmfloat->ch[0]);
			pch[1] = (float*)(pcmhdl->pcmfloat->ch[1]);
			if (pcmhdl->nChannels == 2)
			{
				int i ;
				nread /= pcmhdl->nChannels;
				for (i=0; i<nread; i++)
				{
					pch[0][i] = buffer[2*i];
					pch[1][i] = buffer[2*i + 1];
				}
			}
			else if (pcmhdl->nChannels == 1)
			{
				memcpy (pch[0], buffer, nread*sizeof(float));
			}
			else 
				nread = -1;
		}

	}
	return nread;
}

int
readPCM_data_ieee_double(pcm_reader_data* pcmhdl, int toread)
{
	int nread = -1;
	if (pcmhdl && pcmhdl->wave_in)
	{
		if (pcmhdl->bitspersample != sizeof(double)*8)
		{
			return -1;
		}
		double buffer[DEF_READ_SIZE];
		nread = (DEF_READ_NSAMPLE)>toread?toread:(DEF_READ_NSAMPLE);
		nread *= pcmhdl->nChannels;
		nread = fread (buffer, sizeof(double), nread, pcmhdl->wave_in);
		
		if (nread > 0)
		{
			double *pch[2] ;
			pch[0] = (double*)(pcmhdl->pcmdouble->ch[0]);
			pch[1] = (double*)(pcmhdl->pcmdouble->ch[1]);
			if (pcmhdl->nChannels == 2)
			{
				int i ;
				nread /= pcmhdl->nChannels;
				for (i=0; i<nread; i++)
				{
					pch[0][i] = buffer[2*i];
					pch[1][i] = buffer[2*i + 1];
				}
			}
			else if (pcmhdl->nChannels == 1)
			{
				memcpy (pch[0], buffer, nread*sizeof(double));
			}
			else 
				nread = -1;
		}

	}
	return nread;
}

int
setPCM_file_position(pcm_reader_data* pcmhdl)
{
	int rc = -EINVAL;

	if (pcmhdl)
	{
		WAVE_FILE_INFO_T *wfinfo = NULL;
		wfinfo = (WAVE_FILE_INFO_T*)pcmhdl->info;
		if (wfinfo)
		{
			if (!pcmhdl->wave_in)
			{
				if (wfinfo->name)
				{
					pcmhdl->wave_in = fopen (wfinfo->name,"rb");
				}
			}
			if (pcmhdl->wave_in)
			{
				rc = fseek(pcmhdl->wave_in, wfinfo->datainfo.offset,SEEK_SET);
			}
		}
	}

	return rc;
}

int
setPCM_PcmBuffer (pcm_reader_data *pcmhdl, PcmBuffer *pcm16, PcmBuffer *pcm32, PcmBuffer *pcmfloat, PcmBuffer *pcmdouble)
{
	if (pcmhdl)
	{
		if (pcmhdl->useExtBuffer & EXT_PCM_BUFFER)
		{
			pcmhdl->pcm16 = pcm16;
			pcmhdl->pcm32 = pcm32;
			pcmhdl->pcmfloat = pcmfloat;
			pcmhdl->pcmdouble = pcmdouble;
			return 0;
		}
	}
	return -EINVAL;
}

int	_setPCM_info_from_wavefileinfo(pcm_reader_data *pcmhdl)
{
	int rc = -EINVAL;
	if (pcmhdl && pcmhdl->info)
	{
		WAVE_FILE_INFO_T	*wfinfo = (WAVE_FILE_INFO_T*)pcmhdl->info;
		pcmhdl->num_samples_read = 0;
		pcmhdl->bitspersample = getWAVEBitsPerSample(wfinfo);
		pcmhdl->nChannels = getWAVEChannels(wfinfo);
		pcmhdl->sampleRate = getWAVESampleRate(wfinfo);
		pcmhdl->datalength = getWAVEDataLength (wfinfo);
		if (pcmhdl->nChannels && pcmhdl->bitspersample)
		{
			pcmhdl->numOfSamples = 
				pcmhdl->datalength/(pcmhdl->nChannels*((pcmhdl->bitspersample+7)/8));
		}
		pcmhdl->blockAlign = getWAVEBlockAlign (wfinfo);
		if (WAVE_FORMAT_IEEE_FLOAT == getWAVEFormatTag(wfinfo))
		{
			pcmhdl->is_ieee_float = 1;
		}
		pcmhdl->validBitsPerSample = getWAVEValidBitsPerSample(wfinfo);
		if (pcmhdl->validBitsPerSample != pcmhdl->bitspersample
			|| pcmhdl->validBitsPerSample%8 )
		{
			int i = 0;
			pcmhdl->validBitsMask = 0;
			for ( ; i<pcmhdl->validBitsPerSample; i++)
			{
				pcmhdl->validBitsMask |= (1<<i);
			}
		}
	}

	return rc;
}

int
initPcmBuffer (PcmBuffer *pbuf, int nSamples, int nSize)
{
	int rc = -EINVAL;
	if (pbuf)
	{
		pbuf->ch[0] = realloc(pbuf->ch[0], nSamples * nSize);
		if (!pbuf->ch[0])
		{
			return -ENOMEM;;
		}
		pbuf->ch[1] = realloc(pbuf->ch[1], nSamples * nSize);
		if (!pbuf->ch[1])
		{
			free (pbuf->ch[0]);
			pbuf->ch[0] = NULL;
			return -ENOMEM;;
		}
		pbuf->w = nSize;
		pbuf->n = nSamples;
		pbuf->u = 0;
		return 0;
	}
	return rc;
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

void
resetPcmBuffer (PcmBuffer *pbuf)
{
	if (pbuf)
	{
		if (pbuf->ch[0])
		{
			memset (pbuf->ch[0], 0x00, pbuf->n*pbuf->w);
		}
		if (pbuf->ch[1])
		{
			memset (pbuf->ch[1], 0x00, pbuf->n*pbuf->w);
		}
	}
}

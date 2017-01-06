/***
    wave.c
    Copyright (C) 2016  Park, Sangjun

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//#include "nein/error.h"
#include "nein/wave.h"
#include "nein/debug.h"

static
WAVE_FILE_INFO_T*
allocWAVE_BASE(void)
{
	WAVE_FILE_INFO_T *p = NULL;
	if(NULL==(p = (WAVE_FILE_INFO_T*)malloc(sizeof(WAVE_FILE_INFO_T)))){
		return NULL;
	}
	memset (p,0x00, sizeof(WAVE_FILE_INFO_T));
	return p;
}

static long getFileLength (FILE *fp);
static int  readRIFFHeader (FILE *fp, WAVE_HEADER_T *riff);
static int	readNextChunk (FILE *fp, CHUNK_T *chunk);
static void printChunk(CHUNK_T *chk);

WAVE_FILE_INFO_T*
getWaveInfoFromFile (const char* file)
{
	FILE *ifp = NULL;
	WAVE_FILE_INFO_T *pOut = NULL;
	int  nread = 0;

	if (!file){
		return NULL;
	}
	if (!(ifp=fopen(file, "rb"))){
		NIL_ERROR("File(%s) cannot be opened.\n", file);
		return NULL;
	}

	pOut = getWaveInfo (ifp);
	if (pOut) {
		pOut->name = strdup (file);
	}

	fclose (ifp);

	return pOut;
}

WAVE_FILE_INFO_T*
getWaveInfo (FILE* infp)
{
	WAVE_FILE_INFO_T *pOut = NULL;
	int		nread = 0;
	int		needToRead = 0;

	if (!infp){
		return NULL;
	}
	if (!(pOut=allocWAVE_BASE())){
		NIL_ERROR("Fail to allocate WAVE_FILE_INFO_T.\n");
		return NULL;
	}

	pOut->length = (uint32_t)getFileLength (infp);
	if (pOut->length == 0 || pOut->length == (uint32_t)-1)
	{
		NIL_ERROR("File length is zero.\n");
		goto not_wave_file;
	}
	pOut->isWAVEfile = readRIFFHeader (infp, &(pOut->waveHeader));
	if (pOut->isWAVEfile && 
			pOut->length != (uint32_t)(pOut->waveHeader.riff.chk_size + 8)) 
	{
		pOut->isWAVEfile = 0;
	}
	
	if (pOut->isWAVEfile)
	{
		CHUNK_T	chk;
		int		rc = 0;
		void *p = NULL;
		while (!feof(infp))
		{
			rc = readNextChunk (infp, &chk);
			if (rc>0)
			{
				long	skip_or_read = (long)chk.chk_size;
				if (!strncmp((const char*)chk.CHKIDS, "afsp", 4))
				{ /* temporary.. */
					if (skip_or_read%2) skip_or_read += 1;
				}
				if (!strncmp((const char*)chk.CHKIDS, "fmt ", 4))
				{
					unsigned char buffer[256] = {0, };
					if (skip_or_read > 256) skip_or_read = 256;
					rc = fread (buffer, 1, (size_t)skip_or_read, infp);
					if (rc == skip_or_read)
					{
						//uint16_t fmtTag;
						p = &(pOut->waveInfo);
						memcpy (p, &chk, sizeof(CHUNK_T));
						p += sizeof(CHUNK_T);
						if (skip_or_read > 40) skip_or_read = 40;
						memcpy (p, buffer, skip_or_read);

#if 0
						fmtTag = ((WAVE_PCM_T*)&(pOut->waveInfo))->fmtTag;
						NIL_DEBUG("fmtTag : %04x\n", fmtTag);

						if (WAVE_FORMAT_PCM == fmtTag)
							pOut->waveType = WAVE_PCM_TYPE;
						else if ( WAVE_FORMAT_IEEE_FLOAT == fmtTag
								|| WAVE_FORMAT_ALAW == fmtTag 
								|| WAVE_FORMAT_MULAW == fmtTag)
							pOut->waveType = WAVE_NON_PCM_TYPE;
						else if (WAVE_FORMAT_EXTENSIBLE == fmtTag)
							pOut->waveType = WAVE_EXT_FMT_TYPE;
						else 
							pOut->waveType = 0;
#else
						if (chk.chk_size == 16)
							pOut->waveType = WAVE_PCM_TYPE;
						else if (chk.chk_size == 18)
							pOut->waveType = WAVE_NON_PCM_TYPE;
						else if (chk.chk_size == 40)
							pOut->waveType = WAVE_EXT_FMT_TYPE;
						else
							pOut->waveType = 0;
#endif
					} 
					else
					{
						NIL_ERROR ("Cannot read %ld bytes.\n", skip_or_read);
					}
				}
				else if (!strncmp((const char*)chk.CHKIDS, "fact", 4))
				{
					p = &(pOut->factchk);
					memcpy (p, &chk, sizeof(CHUNK_T));
					//p += sizeof(CHUNK_T);
					rc = fread(&(pOut->factchk.dwSampleLength), sizeof(uint32_t),1,infp);
					NIL_DEBUG ("\tdwSampleLength: %u\n", pOut->factchk.dwSampleLength);
				}
				else if (!strncmp((const char*)chk.CHKIDS, "data", 4))
				{
					p = &(pOut->datainfo);
					memcpy (p, &chk, sizeof(CHUNK_T));
					pOut->datainfo.offset = (uint32_t)ftell(infp);
					if (skip_or_read%2) skip_or_read += 1;
					fseek (infp, skip_or_read, SEEK_CUR);
				}
				else 
				{
					fseek (infp, skip_or_read, SEEK_CUR);
				}
			}
			else 
			{
				break;
			}
		}
		return pOut;
	}
not_wave_file:
	free (pOut);
	return NULL;
}

/* 
1: supported ,
0: not supported 
 */
int
isSupportedWAVEFile (WAVE_FILE_INFO_T* wfinfo)
{
	if (wfinfo) {
		uint16_t	fmtTag = getWAVEFormatTag (wfinfo);
		/* WAVE_FORMAT_PCM || WAVE_FORMAT_IEEE_FLOAT */
		if (WAVE_FORMAT_PCM == fmtTag 
			|| WAVE_FORMAT_IEEE_FLOAT == fmtTag) {
			short nch = getWAVEChannels (wfinfo);
			if (nch == 1 || nch == 2)
				return 1;
		}
	}
	return 0;
}

#define WAVEINFO(type,x) ((type *)&((x)->waveInfo))

uint16_t
getWAVEFormatTag (WAVE_FILE_INFO_T* wfinfo)
{
	if (wfinfo){
		uint16_t fmtTag = ((WAVE_PCM_T*)&(wfinfo->waveInfo))->fmtTag;
		if (fmtTag == WAVE_FORMAT_EXTENSIBLE) {
			WAVE_EXT_FMT_T *extfmt = (WAVE_EXT_FMT_T*)&(wfinfo->waveInfo);
			if (extfmt->fmtTag == WAVE_FORMAT_EXTENSIBLE && extfmt->cbSize == 22) {
				guid_struct *pguid = (guid_struct*)extfmt->subformat;
				fmtTag = (uint16_t)pguid->data.data1;
			}
		}
		return fmtTag;
	}
	return -EINVAL;
}

uint16_t
getWAVEFormatTag_Orig (WAVE_FILE_INFO_T* wfinfo)
{
	if (wfinfo)
	{
		uint16_t fmtTag = ((WAVE_PCM_T*)&(wfinfo->waveInfo))->fmtTag;
		return fmtTag;
	}
	return -EINVAL;
}

short
getWAVEChannels (WAVE_FILE_INFO_T* wfinfo)
{
	short nChannels = 0;
	if (!wfinfo){
		return -EINVAL;
	}
	nChannels = ((WAVE_PCM_T*)&wfinfo->waveInfo)->nChannels;
	return nChannels;
}

int
getWAVESampleRate (WAVE_FILE_INFO_T* wfinfo)
{
	if (wfinfo)
	{
		return ((WAVE_PCM_T*)&wfinfo->waveInfo)->nSamplePerSec;
	}
	return -EINVAL;
}

int
getWAVEBlockAlign (WAVE_FILE_INFO_T* wfinfo)
{
	if (wfinfo)
	{
		return (int)(((WAVE_PCM_T*)&wfinfo->waveInfo)->nBlockAlign);
	}
	return -EINVAL;
}

short
getWAVEBitsPerSample (WAVE_FILE_INFO_T* wfinfo)
{
	short bitspersample = 0;
	if (!wfinfo){
		return -EINVAL;
	}
	if (WAVE_PCM_TYPE == wfinfo->waveType){
		bitspersample = ((WAVE_PCM_T*)&(wfinfo->waveInfo))->wBitsPerSample;
	} else if (WAVE_NON_PCM_TYPE == wfinfo->waveType) {
		bitspersample = ((WAVE_NON_PCM_T*)&(wfinfo->waveInfo))->wBitsPerSample;
	} else if (WAVE_EXT_FMT_TYPE == wfinfo->waveType) {
		bitspersample = ((WAVE_EXT_FMT_T*)&(wfinfo->waveInfo))->wBitsPerSample;
		if (bitspersample == 0) {
			bitspersample = ((WAVE_EXT_FMT_T*)&wfinfo->waveInfo)->Samples.wSamplesPerBlock;
		}
	} else {
		NIL_DEBUG("%s:%d Not supported wave file format.\n",__FILE__,__LINE__);
	}
	return bitspersample;
}

short
getWAVEValidBitsPerSample (WAVE_FILE_INFO_T* wfinfo)
{
	short bitspersample = -1;
	if (!wfinfo)
	{
		return -EINVAL;
	}

	if (WAVE_PCM_TYPE == wfinfo->waveType)
	{
		bitspersample = ((WAVE_PCM_T*)&(wfinfo->waveInfo))->wBitsPerSample;
	} 
	else if (WAVE_NON_PCM_TYPE == wfinfo->waveType)
	{
		bitspersample = ((WAVE_NON_PCM_T*)&(wfinfo->waveInfo))->wBitsPerSample;
	}
	else if (WAVE_EXT_FMT_TYPE == wfinfo->waveType)
	{
		short validBitsPerSample = 0;
		WAVE_EXT_FMT_T *extfmt = (WAVE_EXT_FMT_T*)&(wfinfo->waveInfo);
		bitspersample = extfmt->wBitsPerSample;
		if (extfmt->fmtTag == WAVE_FORMAT_EXTENSIBLE && extfmt->cbSize == 22) {
			validBitsPerSample = extfmt->Samples.wValidBitsPerSample;
		}
		if (bitspersample != validBitsPerSample)
		{
			bitspersample = validBitsPerSample;
		}
	}
	else
	{
		NIL_DEBUG("%s:%d Not supported wave file format.\n", __FILE__,__LINE__);
	}
	return bitspersample;
}

int 
getWAVEDataLength (WAVE_FILE_INFO_T *wfinfo)
{
	if (wfinfo){
		return wfinfo->datainfo.data.chk_size;
	}
	return -EINVAL;
}

long 
getWAVEDataOffset (WAVE_FILE_INFO_T*	waveinfo)
{
	if (waveinfo)
	{
		return waveinfo->datainfo.offset;
	}
	return 0L;
}

void
freeWaveInfo (WAVE_FILE_INFO_T *info)
{
	if (info)
	{
		if (info->name) free(info->name);
		free (info);
	}
}

long getFileLength (FILE *fp)
{
	int length = 0;
	if (fp) 
	{
		if (0==fseek (fp, 0L, SEEK_END))
		{
			length = ftell (fp);
			fseek (fp, 0L, SEEK_SET);
		}
	}
	return length;
}

int readRIFFHeader (FILE* fp, WAVE_HEADER_T *riff_tag)
{
	int rc = 0;
	if (fp) 
	{
		WAVE_HEADER_T riff;
		/* rewind */
		long currPos = ftell (fp);
		if (currPos>0) 
			fseek (fp, 0L, SEEK_SET);
		rc = fread (&riff, sizeof(WAVE_HEADER_T), 1, fp);
		if (rc == 1)
		{
			printChunk(&riff.riff);
			NIL_DEBUG("ID: %4.4s, %x\n", riff.waveid.WAVEIDS, riff.waveid.WAVEID);
			if ( strncmp((const char*)riff.riff.CHKIDS, "RIFF", 4) 
				|| strncmp((const char*)riff.waveid.WAVEIDS, "WAVE", 4) )
			{
				rc =0;
			}
			else 
			{
				rc = 1;
				if (riff_tag)
				{
					memcpy (riff_tag, &riff, sizeof(WAVE_HEADER_T));
				}
			}
		}
		else 
		{
			rc = 0;
		}
	} 
	return rc;
}

int readNextChunk (FILE *fp, CHUNK_T *chunk)
{
	int rc = -1;
	if (fp && chunk)
	{
		rc = fread (chunk, sizeof(CHUNK_T), 1, fp);
		if (rc == 1)
		{
#if !defined(NDEBUG) && !defined(NIL_DEBUG_OFF)
			if (chunk->ID.chkids[0]=='\0')
			{
				printf ("ID: %c%c%c%cs, %x\n"
						, chunk->ID.chkids[0]
						, chunk->ID.chkids[1]
						, chunk->ID.chkids[2]
						, chunk->ID.chkids[3]
						, chunk->CHKID);
			}
			else 
			{
				printf ("ID: %4.4s, %x\n", chunk->CHKIDS, chunk->CHKID);
			}
			printf ("Length: %u\n", chunk->chk_size);
#endif
			rc = (int)chunk->chk_size;
		}
	}
	return rc;
}

static void printWaveFmtInfo (void *p)
{
	WAVE_PCM_T *pcm = (WAVE_PCM_T*)p;
	printf ("\tID: %4.4s, %x\n", pcm->fmt.CHKIDS, pcm->fmt.CHKID);
	printf ("\tLength: %u\n", pcm->fmt.chk_size);
	printf ("\tFormatTag: 0x%04x\n", pcm->fmtTag);
	printf ("\tNo. Channels: %hd\n", pcm->nChannels);
	printf ("\tSample/Sec: %d\n", pcm->nSamplePerSec);
	printf ("\tAvgBytes/Sec: %d\n", pcm->nAvgBytesPerSec);
	printf ("\tBlcak Align: %hd\n", pcm->nBlockAlign);
	printf ("\tBits/Sample: %hd\n", pcm->wBitsPerSample);
	if (pcm->fmt.chk_size > 16)
	{
		WAVE_NON_PCM_T	*nonpcm = (WAVE_NON_PCM_T*)p;
		printf ("\tcbSize: %hd\n", nonpcm->cbSize);
	}
	if (pcm->fmt.chk_size > 18)
	{
		int i = 0;
		WAVE_EXT_FMT_T *extfmt = (WAVE_EXT_FMT_T*)p;
		printf("\tValidBitsPerSample: %hd\n", extfmt->Samples.wValidBitsPerSample);
		printf("\tChannel mask: h%08x\n", extfmt->dwChannelMask);
		printf("\tSubFormat: ");
		for ( ; i<16; i++) {
			printf("%02x%s",extfmt->subformat[i],i==15?"\n":" ");
		}
		{
			guid_struct *pguid = (guid_struct*)extfmt->subformat;
			char *tmp = NULL;
			if (WAVE_FORMAT_PCM == pguid->data.data1)
			{
				static char WAVE_PCM[] = "PCM";
				tmp = WAVE_PCM;
			}
			else if (WAVE_FORMAT_IEEE_FLOAT == pguid->data.data1)
			{
				static char WAVE_IEEE_FLOAT[] = "IEEE_FLOAT";
				tmp = WAVE_IEEE_FLOAT;
			}
			else if (WAVE_FORMAT_ALAW == pguid->data.data1)
			{
				static char WAVE_ALAW[] = "ALAW";
				tmp = WAVE_ALAW;
			}
			else if (WAVE_FORMAT_MULAW == pguid->data.data1)
			{
				static char WAVE_MULAW[] = "MULAW";
				tmp = WAVE_MULAW;
			}
			else if (WAVE_FORMAT_ADPCM == pguid->data.data1)
			{
				static char WAVE_ADPCM[] = "ADPCM";
				tmp = WAVE_ADPCM;
			}
			else if (WAVE_FORMAT_MPEG == pguid->data.data1)
			{
				static char WAVE_MPEG[] = "MPEG";
				tmp = WAVE_MPEG;
			} else {
				static char WAVE_NON[] = "NONE";
				tmp = WAVE_NON;
			}
			printf("\t\tType: %s\n",  tmp);
		}
	}
}

void 
printWaveInfo(WAVE_FILE_INFO_T *info)
{
	if (info)
	{
		printf("File: %s\n", info->name);
		printf("Length: %u\n", info->length);
		printf ("Is it a wave file? %s\n", info->isWAVEfile?"Yes":"No");
		if (info->isWAVEfile) 
		{
			printChunk(&(info->waveHeader.riff));
			printf ("ID: [%4.4s] (0x%x)\n"
				, info->waveHeader.waveid.WAVEIDS, info->waveHeader.waveid.WAVEID);
			printWaveFmtInfo ( &(info->waveInfo) );
			//printChunk(&(info->datainfo.data));
			printf("ID: [%4.4s] (0x%x), Length: %d\n"
					, info->datainfo.data.CHKIDS
					, info->datainfo.data.CHKID
					, info->datainfo.data.chk_size);
			printf("offset of data: %u\n", info->datainfo.offset);
			if (info->factchk.fact.chk_size)
			{
				printChunk(&(info->factchk.fact));
				printf ("FACT\tdwSampleLength: %u\n"
						, info->factchk.dwSampleLength);
			}
		}
	}
	return ;
}

void
printChunk(CHUNK_T *chk)
{
	NIL_DEBUG("ID: [%4.4s] (0x%x), Length: %d\n",
			chk->CHKIDS, chk->CHKID, chk->chk_size );
}


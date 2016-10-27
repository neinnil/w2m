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
		fprintf(stderr,"File(%s) cannot be opened.\n", file);
		return NULL;
	}

	pOut = getWaveInfo (ifp);

	fclose (ifp);

	return pOut;
}

WAVE_FILE_INFO_T*
getWaveInfo (FILE* infp)
{
	WAVE_FILE_INFO_T *pOut = NULL;
	int  nread = 0;
	int		needToRead = 0;

	if (!infp){
		return NULL;
	}
	if (!(pOut=allocWAVE_BASE())){
		fprintf(stderr,"Fail to allocate WAVE_FILE_INFO_T.\n");
		fclose (infp);
		return NULL;
	}
/* assume PCM format */
	printf("sizeof(WAVE_PCM_T): %ld\n", sizeof(WAVE_PCM_T));
	nread = fread((unsigned char*)&(pOut->waveInfo), 1, sizeof(WAVE_PCM_T), infp);
	if (nread!=sizeof(WAVE_PCM_T)){
		fprintf(stderr,"Fail to read from file\n");
		free(pOut);
		pOut = NULL;
	} else {
		WAVE_PCM_T	*pcm = NULL;
		pcm = (WAVE_PCM_T*)&(pOut->waveInfo);
		/* first check RIFF */
		if (strncmp((const char*)pcm->riff.CHKIDS, "RIFF", 4)) {
			fprintf(stderr,"There is no RIFF header.\n");
			goto not_wave_file;
		}

		if (strncmp((const char*)pcm->waveid.WAVEIDS,"WAVE", 4)) {
			fprintf(stderr,"There is no WAVE header.\n");
			goto not_wave_file;
		}

		if (pcm->fmt.chk_size == 16) {
			pOut->waveType = WAVE_PCM_TYPE;
		} else if (pcm->fmt.chk_size == 18) {
			pOut->waveType = WAVE_NON_PCM_TYPE;
			printf("sizeof(WAVE_NON_PCM_T): %ld\n", sizeof(WAVE_NON_PCM_T));
			needToRead = sizeof(WAVE_NON_PCM_T) - sizeof(WAVE_PCM_T);
		} else if (pcm->fmt.chk_size == 40) {
			pOut->waveType = WAVE_EXT_FMT_TYPE;
			printf("sizeof(WAVE_EXT_FMT_T): %ld\n", sizeof(WAVE_EXT_FMT_T));
			needToRead = sizeof(WAVE_EXT_FMT_T) - sizeof(WAVE_PCM_T);
		}
		if (needToRead > 0) {
			printf("Read More %d\n", needToRead);
			unsigned char *p = (unsigned char*)&(pOut->waveInfo);
			p += sizeof(WAVE_PCM_T);
			nread = fread(p, sizeof(unsigned char), needToRead, infp);
			if (nread != needToRead){
				fprintf(stderr,"ToDo: handle error case.\n");
			}
		}
	}
	return pOut;
not_wave_file:
	free (pOut);
	return NULL;
}

/* 
1: supported ,
0: not supported 
*/
int
isSupportedWAVEFile (WAVE_FILE_INFO_T* wavefile)
{
	if (!wavefile) {
		return 0;
	}
	/* WAVE_FORMAT_PCM || WAVE_FORMAT_IEEE_FLOAT */
	if (WAVE_FORMAT_PCM == getWAVEFormatTag(wavefile)) {
		return 1;
	}
	return 0;
}

short
getWAVEFormatTag (WAVE_FILE_INFO_T* wavefile)
{
	if (!wavefile){
		return -EINVAL;
	}
	return (((WAVE_PCM_T*)&wavefile->waveInfo)->fmtTag);
}

short
getWAVEChannels (WAVE_FILE_INFO_T* wavefile)
{
	short nChannels = 0;
	if (!wavefile){
		return -EINVAL;
	}
	nChannels = ((WAVE_PCM_T*)&wavefile->waveInfo)->nChannels;
	return nChannels;
}

int
getWAVESampleRate (WAVE_FILE_INFO_T* wavefile)
{
	int sampleRate = 0; 
	if (!wavefile){
		return -EINVAL;
	}
	if (WAVE_PCM_TYPE == wavefile->waveType){
		sampleRate = ((WAVE_PCM_T*)&wavefile->waveInfo)->nSamplePerSec;
	} else if (WAVE_NON_PCM_TYPE == wavefile->waveType) {
		sampleRate = ((WAVE_NON_PCM_T*)&wavefile->waveInfo)->nSamplePerSec;
	} else if (WAVE_EXT_FMT_TYPE == wavefile->waveType) {
		sampleRate = ((WAVE_EXT_FMT_T*)&wavefile->waveInfo)->nSamplePerSec;
	} else {
		fprintf(stderr,"Not supported wave file format.\n");
	}
	return sampleRate;
}

#define WAVEINFO(type,x) ((type *)&((x)->waveInfo))

short
getWAVEBitsPerSample (WAVE_FILE_INFO_T* wavefile)
{
	short bitpersample = 0;
	if (!wavefile){
		return -EINVAL;
	}
	if (WAVE_PCM_TYPE == wavefile->waveType){
		bitpersample = ((WAVE_PCM_T*)&wavefile->waveInfo)->wBitsPerSample;
	} else if (WAVE_NON_PCM_TYPE == wavefile->waveType) {
		bitpersample = ((WAVE_NON_PCM_T*)&wavefile->waveInfo)->wBitsPerSample;
	} else if (WAVE_EXT_FMT_TYPE == wavefile->waveType) {
		bitpersample = ((WAVE_EXT_FMT_T*)&wavefile->waveInfo)->wBitsPerSample;
	} else {
		fprintf(stderr,"Not supported wave file format.\n");
	}
	return bitpersample;
}

int
getWAVEDataLength (WAVE_FILE_INFO_T *wavefile)
{
	CHUNK_T	*data = NULL;
	if (!wavefile){
		return -EINVAL;
	}
	if (WAVE_PCM_TYPE == wavefile->waveType) {
//		WAVE_PCM_T *pcm = &(wavefile->waveInfo);
//		data = &(pcm->data);
		data = &(((WAVE_PCM_T*)&(wavefile->waveInfo))->data);
	} else if (WAVE_NON_PCM_TYPE == wavefile->waveType) {
//		WAVE_NON_PCM_T *nonpcm = &(wavefile->waveInfo);
//		data = &(nonpcm->data);
		data = &(((WAVE_PCM_T*)&(wavefile->waveInfo))->data);
	} else if (WAVE_EXT_FMT_TYPE == wavefile->waveType) {
//		WAVE_EXT_FMT_T *ext = &(wavefile->waveInfo);
//		data = &(ext->data);
		data = &((WAVE_EXT_FMT_T*)&wavefile->waveInfo)->data;
	} else {
		fprintf(stderr,"Not Supported sort of wave file.\n");
		return -EINVAL;
	}

	return data->chk_size;
}

static void printChunk(CHUNK_T *chk)
{
	printf ("ID: [%4.4s] (0x%x), Length: %d\n",
			chk->CHKIDS, chk->CHKID, chk->chk_size );
}

void 
printWaveInfo(WAVE_FILE_INFO_T *info)
{
	if (NULL == info) return;
	WAVE_PCM_T	*pcm = (WAVE_PCM_T*)&info->waveInfo;

	printChunk(&pcm->riff);
	printf ("ID: [%4.4s] (0x%x)\n", pcm->waveid.WAVEIDS, pcm->waveid.WAVEID);
	printChunk(&pcm->fmt);
	printf ("FormatTag: 0x%04x\n", pcm->fmtTag);
	printf ("No. Channels: %hd\n", pcm->nChannels);
	printf ("Sample/Sec: %d\n", pcm->nSamplePerSec);
	printf ("AvgBytes/Sec: %d\n", pcm->nAvgBytesPerSec);
	printf ("Blcak Align: %hd\n", pcm->nBlockAlign);
	printf ("Bits/Sample: %hd\n", pcm->wBitsPerSample);

	if (WAVE_PCM_TYPE == info->waveType) {
		printf ("Normal PCM format\n");
		printChunk(&pcm->data);
	} else if (WAVE_NON_PCM_TYPE == info->waveType) {
		WAVE_NON_PCM_T *nonpcm = (WAVE_NON_PCM_T*)&info->waveInfo;
		printf("cb Size: %hd\n", nonpcm->cbSize);
		printChunk(&nonpcm->fact);
		printf ("SampleLength: %d\n", nonpcm->dwSampleLength);
		printChunk(&nonpcm->data);
	} else if (WAVE_EXT_FMT_TYPE == info->waveType) {
		int i=0;
		WAVE_EXT_FMT_T *extfmt = (WAVE_EXT_FMT_T*)&info->waveInfo;
		printf("cb Size: %hd\n", extfmt->cbSize);
		printf("ValidBitsPerSample: %hd\n", extfmt->wValidBitsPerSample);
		printf("Channel mask: h%08x\n", extfmt->dwChannelMask);
		printf("SubFormat: ");
		for ( ; i<16; i++) {
			printf("%02x%s",extfmt->subformat[i],i==15?"\n":" ");
		}
		printChunk(&extfmt->fact);
		printf ("SampleLength: %d\n", extfmt->dwSampleLength);
		printChunk(&extfmt->data);
	} else {
		fprintf (stderr, "Ooops!!!");
	}
}

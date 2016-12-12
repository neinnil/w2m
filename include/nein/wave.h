/***
    nein_wave.h
    Copyright (C) 2016  Park, Sangjun (neinnil@gmail.com)

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
#ifndef _NEIN_WAVE_H
#define _NEIN_WAVE_H

#include <stdint.h>

#include "nein/wave_fmt.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
typedef struct _CHUNK_T {
	union {
		uint8_t		chkids[4];
		uint32_t	chkid;
	}ID;
#define CHKIDS ID.chkids
#define CHKID  ID.chkid
	uint32_t		chk_size;
} CHUNK_T;

typedef struct _RIFF_T {
	uint32_t	ckID;
	uint32_t	cksize;
	uint32_t	waveid;
} RIFF_CHUNK_T;

typedef struct _WAVEID_T {
	union {
		uint8_t		waveids[4];
		uint32_t	waveid;
	}WID;
#define WAVEIDS WID.waveids
#define WAVEID  WID.waveid
} WAVEID_T;

typedef struct _PCM_Format_T {
	CHUNK_T	fmt;
	uint16_t	fmtTag;
	uint16_t	nChannels;
	uint32_t	nSamplePerSec;
	uint32_t	nAvgBytesPerSec;
	uint16_t	nBlockAlign;
	uint16_t	BitsPerSample;
} PCM_Format_t;

typedef struct _WAVEINFO_BASE_T {
	CHUNK_T		riff;
	WAVEID_T	waveid;
	CHUNK_T		fmt;
	uint16_t	fmtTag;
	uint16_t	nChannels;
	uint32_t	nSamplePerSec;
	uint32_t	nAvgBytesPerSec;
	uint16_t	nBlockAlign;
	uint16_t	BitsPerSample;
} WAVEINFO_BASE_T;

typedef struct _FACT_CHUNK_T {
	CHUNK_T		fact;
	uint32_t	dwSampleLength;
} FACT_CHUNK_T;

typedef struct _DATA_CHUNK_T {
	CHUNK_T		data;
#define datalen data.chk_size
	unsigned char sampleData[1];

} DATA_CHUNK_T;

typedef struct _WAVE_EXT_T {
	uint16_t	wValidBitsPerSample;
	uint32_t	dwChannelMask;
	union {
		uint64_t	guid[2];
		uint8_t		guids[16]; 
	} subFormat;
}WAVE_EXT_T;

typedef struct _WAVE_PCM_T {
	CHUNK_T		riff;
	WAVEID_T	waveid;
	CHUNK_T		fmt;
	uint16_t	fmtTag;
	uint16_t	nChannels;
	uint32_t	nSamplePerSec;
	uint32_t	nAvgBytesPerSec;
	uint16_t	nBlockAlign;
	uint16_t	wBitsPerSample;
	CHUNK_T		data;
	/* unsigned char samples[1]; */
} WAVE_PCM_T;

typedef struct _WAVE_NON_PCM_T {
	CHUNK_T		riff;
	WAVEID_T	waveid;
	CHUNK_T		fmt; /* chunk size: 18 */
	uint16_t	fmtTag;
	uint16_t	nChannels;
	uint32_t	nSamplePerSec;
	uint32_t	nAvgBytesPerSec;
	uint16_t	nBlockAlign;
	uint16_t	wBitsPerSample;
	uint16_t	cbSize; /* value: 0 */
	CHUNK_T		fact;
	uint32_t	dwSampleLength;
	CHUNK_T		data;
	/* unsigned char samples[1]; */
} WAVE_NON_PCM_T;

typedef struct _WAVE_EXT_FMT_T {
	CHUNK_T			riff;
	WAVEID_T		waveid;
	CHUNK_T			fmt; /* chunk size: 40 */
	uint16_t		fmtTag;
	uint16_t		nChannels;
	uint32_t		nSamplePerSec;
	uint32_t		nAvgBytesPerSec;
	uint16_t		nBlockAlign;
	uint16_t		wBitsPerSample;
	uint16_t		cbSize; /* value: 22 */
	uint16_t		wValidBitsPerSample;
	uint32_t		dwChannelMask;
	uint8_t		 	subformat[16];
	CHUNK_T			fact;
	uint32_t		dwSampleLength;
	CHUNK_T			data;
	/* unsigned char samples[1]; */
} WAVE_EXT_FMT_T;

#define SZ_DUMMY sizeof(WAVE_EXT_FMT_T)
typedef struct _WAVE_DUMMY_T {
	unsigned char rawdata[SZ_DUMMY];
} WAVE_DUMMY_T;

typedef struct _WAVE_FILE_INFO_T {
	char*			path;
	int				waveType; /* PCM, non-PCM, Extensible format*/
#define WAVE_PCM_TYPE	0x0001
#define	WAVE_NON_PCM_TYPE	0x0002
#define WAVE_EXT_FMT_TYPE	0x0003
	WAVE_DUMMY_T	waveInfo;
} WAVE_FILE_INFO_T;

#pragma pack()

#define FREEWAVEFILEINFO(x) \
	do { \
		if (!!(x)) {      \
			free ((x));   \
			((x)) = NULL; \
		} } while(0)



/* getWaveInfo, getWaveInfoFromFile */
extern WAVE_FILE_INFO_T* getWaveInfoFromFile(const char* file);
extern WAVE_FILE_INFO_T* getWaveInfo(FILE *infp);

extern int	 isSupportedWAVEFile(WAVE_FILE_INFO_T* waveinfo);
extern short getWAVEFormatTag(WAVE_FILE_INFO_T* waveinfo);
extern short getWAVEChannels(WAVE_FILE_INFO_T* waveinfo);
extern int	 getWAVESampleRate(WAVE_FILE_INFO_T* waveinfo);
extern short getWAVEBitsPerSample(WAVE_FILE_INFO_T* waveinfo);
extern int	 getWAVEDataLength (WAVE_FILE_INFO_T*	waveinfo);

//extern int	 isWAVE_IEEE_FLOAT(WAVE_FILE_INFO_T *waveinfo);
//extern int	 isWAVE_EIGHTBITSTREAM(WAVE_FILE_INFO_T *waveinfo);

/* for debugging */
void printWaveInfo(WAVE_FILE_INFO_T *info);
/**
void printWaveInfo(WAVEINFO_T *);
void printInt32(FILE* infp, const char *name);
void printInt16(FILE* infp, const char *name);
void print4Bytes(FILE* infp, const char *name);
**/
#ifdef __cplusplus
}
#endif

#endif /* NIL_WAVE_H */

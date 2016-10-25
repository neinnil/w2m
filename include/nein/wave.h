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

#ifdef __cplusplus
extern "C" {
#endif
/* wave format tag id: from rfc2361 */
#define WAVE_FORMAT_UNKNOWN				0x0000
#define WAVE_FORMAT_PCM					0x0001
#define WAVE_FORMAT_ADPCM				0x0002
#define WAVE_FORMAT_IEEE_FLOAT			0x0003
#define WAVE_FORMAT_VSELP				0x0004
#define WAVE_FORMAT_IBM_CVSD			0x0005
#define WAVE_FORMAT_ALAW				0x0006
#define WAVE_FORMAT_MULAW				0x0007
#define WAVE_FORMAT_OKI_ADPCM			0x0010
#define WAVE_FORMAT_DVI_ADPCM			0x0011
#define WAVE_FORMAT_MEDIASPACE_ADPCM	0x0012
#define WAVE_FORMAT_SIERRA_ADPCM		0x0013
#define WAVE_FORMAT_G723_ADPCM			0x0014
#define WAVE_FORMAT_DIGISTD				0x0015
#define WAVE_FORMAT_DIGIFIX				0x0016
#define WAVE_FORMAT_DIALOGIC_OKI_ADPCM	0x0017
#define WAVE_FORMAT_MEDIAVISION_ADPCM	0x0018
#define WAVE_FORMAT_CU_CODEC			0x0019
#define WAVE_FORMAT_YAMAHA_ADPCM		0x0020
#define WAVE_FORMAT_SONARC				0x0021
#define WAVE_FORMAT_DSPGROUP_TRUESPEECH	0x0022
#define WAVE_FORMAT_ECHOSC1				0x0023
#define WAVE_FORMAT_AUDIOFILE_AF36		0x0024
#define WAVE_FORMAT_APTX				0x0025
#define WAVE_FORMAT_AUDIOFILE_AF10		0x0026
#define WAVE_FORMAT_PROSODY_1612		0x0027
#define WAVE_FORMAT_LRC					0x0028
#define WAVE_FORMAT_DOLBY_AC2			0x0030
#define WAVE_FORMAT_GSM610				0x0031
#define WAVE_FORMAT_MSNAUDIO			0x0032
#define WAVE_FORMAT_ANTEX_ADPCME		0x0033
#define WAVE_FORMAT_CONTROL_RES_VQLPC	0x0034
#define WAVE_FORMAT_DIGIREAL			0x0035
#define WAVE_FORMAT_DIGIADPCM			0x0036
#define WAVE_FORMAT_CONTROL_RES_CR10	0x0037
#define	WAVE_FORMAT_NMS_VBXADPCM		0x0038
#define WAVE_FORMAT_ROLAND_RDAC			0x0039
#define WAVE_FORMAT_ECHOSC3				0x003A
#define WAVE_FORMAT_ROCKWELL_ADPCM		0x003B
#define WAVE_FORMAT_ROCKWELL_DIGITALK	0x003C
#define WAVE_FORMAT_XEBEC				0x003D
#define WAVE_FORMAT_G721_ADPCM			0x0040
#define WAVE_FORMAT_G728_CELP			0x0041
#define WAVE_FORMAT_MSG723				0x0042
#define WAVE_FORMAT_MPEG				0x0050
#define WAVE_FORMAT_RT24				0x0052
#define WAVE_FORMAT_PAC					0x0053
#define WAVE_FORMAT_MPEGLAYER3			0x0055
#define WAVE_FORMAT_LUCENT_G723			0x0059
#define WAVE_FORMAT_CIRRUS				0x0060
#define WAVE_FORMAT_ESPCM				0x0061
#define WAVE_FORMAT_VOXWARE				0x0062
#define WAVE_FORMAT_CANOPUS_ATRAC		0x0063
#define WAVE_FORMAT_G726_ADPCM			0x0064
#define WAVE_FORMAT_G722_ADPCM			0x0065
#define WAVE_FORAT_DSAT					0x0066
#define WAVE_FORAT_DSAT_DISPLAY			0x0067
#define WAVE_FORMAT_EXTENSIBLE			0xFFFE

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
	uint16_t		fmtTag;
	uint16_t		nChannels;
	uint32_t			nSamplePerSec;
	uint32_t			nAvgBytesPerSec;
	uint16_t		nBlockAlign;
	uint16_t		wBitsPerSample;
	CHUNK_T		data;
	/* unsigned char samples[1]; */
} WAVE_PCM_T;

typedef struct _WAVE_NON_PCM_T {
	CHUNK_T		riff;
	WAVEID_T	waveid;
	CHUNK_T		fmt; /* chunk size: 18 */
	uint16_t		fmtTag;
	uint16_t		nChannels;
	uint32_t			nSamplePerSec;
	uint32_t			nAvgBytesPerSec;
	uint16_t		nBlockAlign;
	uint16_t		wBitsPerSample;
	uint16_t		cbSize; /* value: 0 */
	CHUNK_T		fact;
	uint32_t			dwSampleLength;
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

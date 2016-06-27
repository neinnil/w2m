#ifndef _NEIN_WAVE_H
#define _NEIN_WAVE_H

#ifdef __cplusplus
extern "C" {
#endif

#define WAVE_FORMAT_PCM			0x0001
#define WAVE_FORMAT_IEEE_FLOAT	0x0003
#define WAVE_FORMAT_EXTENSIBLE	0xFFFE

typedef struct _CHUNK_T {
	union {
		char chkids[4];
		int	 chkid;
	}ID;
#define CHKIDS ID.chkids
#define CHKID  ID.chkid
	int	chk_size;
} CHUNK_T;

typedef struct _RIFF_T {
	int ckID;
	int	cksize;
	int	waveid;
} RIFF_CHUNK_T;

typedef struct _WAVEID_T {
	union {
		char waveids[4];
		int	 waveid;
	}WID;
#define WAVEIDS WID.waveids
#define WAVEID  WID.waveid
} WAVEID_T;

typedef struct _PCM_Format_T {
	CHUNK_T	fmt;
	short fmtTag;
	short nChannels;
	int	  nSamplePerSec;
	int	  nAvgBytesPerSec;
	short nBlockAlign;
	short BitsPerSample;
} PCM_Format_t;

typedef struct _WAVEINFO_BASE_T {
	CHUNK_T	riff;
	WAVEID_T waveid;
	CHUNK_T	fmt;
	short fmtTag;
	short nChannels;
	int	  nSamplePerSec;
	int	  nAvgBytesPerSec;
	short nBlockAlign;
	short BitsPerSample;
} WAVEINFO_BASE_T;

typedef struct _FACT_CHUNK_T {
	CHUNK_T fact;
	int		dwSampleLength;
} FACT_CHUNK_T;

typedef struct _DATA_CHUNK_T {
	CHUNK_T	data;
#define datalen data.chk_size
} DATA_CHUNK_T;

typedef struct _WAVE_EXT_T {
	short wValidBitsPerSample;
	int	  dwChannelMask;
	union {
		unsigned long long guid;
		char			   guids[16]; 
	} subFormat;
}WAVE_EXT_T;

typedef struct _WAVE_PCM_T {
	WAVEINFO_BASE_T base;
	DATA_CHUNK_T	data;
} WAVE_PCM_T;

typedef struct _WAVEINFO_T {
	WAVEINFO_BASE_T	base;
	FACT_CHUNK_T	fact;
	DATA_CHUNK_T	data;

} WAVEINFO_T;

/* getWaveInfo, getWaveInfoFromFile */
extern WAVEINFO_BASE_T* getWaveInfoFromFile(const char* file);
extern WAVEINFO_BASE_T* getWaveInfo(FILE *infp);
extern int	isSupportedWAVEFile(WAVEINFO_BASE_T* waveinfo);
extern short getWAVEFormatTag(WAVEINFO_BASE_T* waveinfo);
extern short getWAVEChannels(WAVEINFO_BASE_T* waveinfo);
extern short getWAVESampleRate(WAVEINFO_BASE_T* waveinfo);
extern short getWAVEBitsPerSample(WAVEINFO_BASE_T* waveinfo);
extern int	 getWAVEDataLength (DATA_CHUNK_T	*data);
extern WAVE_PCM_T* convtWAVEBASE2PCM(WAVEINFO_BASE_T* waveinfo);

/* for debugging */
/** move following functions to uni test. 
void printWaveInfo(WAVEINFO_T *);
void printInt32(FILE* infp, const char *name);
void printInt16(FILE* infp, const char *name);
void print4Bytes(FILE* infp, const char *name);
**/
#ifdef __cplusplus
}
#endif

#endif /* NIL_WAVE_H */

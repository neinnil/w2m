/*
 *  nein_pcm.h
 *  Copyright (C) 2016  Park, Sangjun
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _NEIN_PCM_H
#define _NEIN_PCM_H

#include <stdio.h> /* for FILE* */
#ifdef __cplusplus
extern "C" {
#endif

struct _PcmBuffer {
	void	*ch[2];			/* buffer for each channel */
	int		w;				/* sample width */
	int		n;				/* number samples allocated */
	int		u;				/* number samples used */
#if 0 /* just pcm data */
	int		skip_start;		/* number samples to ignore at the beginning */
	int		skip_end;		/* number samples to ignore at the end */
#endif
};

typedef struct _PcmBuffer PcmBuffer;

typedef struct _PCM_READER_DATA_T
{
	short			nChannels;
	uint32_t		sampleRate;
	uint32_t		datalength;
	int				bitspersample;
	int				blockAlign;
	int				validBitsPerSample;
	int				validBitsMask;
	int				swapbytes;
	int				is_unsigned_8bit;
	int				is_ieee_float;
	int				numOfSamples;
	unsigned int	num_samples_read;
	//int				count_samples;
	FILE*			wave_in;
	int				useExtBuffer;
#define EXT_PCM_BUFFER	0x01
#define EXT_WAVE_INFO	0x02
	PcmBuffer		*pcm32;
	PcmBuffer		*pcm16;
	PcmBuffer		*pcmfloat;
	PcmBuffer		*pcmdouble;
	size_t			in_id3v2_size;
	unsigned char*	in_id3v2_tag;
	void*			info; /* pointer to WAVE_FILE_INFO_T */
} pcm_reader_data;

extern int initPCM_Reader(pcm_reader_data **pcmhandle, int useExtBuffer);
extern void freePCM_Reader(pcm_reader_data *pcmhandle);
extern int setPCM_OpenWaveFile(pcm_reader_data *pcmhandle, const char *file);
extern int setPCM_WaveFile(pcm_reader_data *pcmhandle, FILE *infp);
extern int setPCM_FromWaveInfo(pcm_reader_data* pcmhandle, FILE *infp, void* waveinfo);

extern int setPCM_file_position(pcm_reader_data* pcmhandle);

extern int setPCM_PcmBuffer (pcm_reader_data *pcmhandle, PcmBuffer *pcm16, PcmBuffer *pcm32, PcmBuffer *pcmfloat, PcmBuffer *pcmdouble);
extern int readPCM_data(pcm_reader_data* pcmhandle, int toread);
extern int readPCM_data_int(pcm_reader_data* pcmhandle, int toread);
extern int readPCM_data_uint8(pcm_reader_data* pcmhandle, int toread);
extern int readPCM_data_short(pcm_reader_data* pcmhandle, int toread);
extern int readPCM_data_ieee_float(pcm_reader_data* pcmhandle, int toread);
extern int readPCM_data_ieee_double(pcm_reader_data* pcmhandle, int toread);

extern int	initPcmBuffer (PcmBuffer *pbuf, int nSamples, int nSize);
extern void freePcmBuffer (PcmBuffer *pbuf);
extern void	resetPcmBuffer (PcmBuffer *pbuf);

	
#ifdef __cplusplus
}
#endif

#endif

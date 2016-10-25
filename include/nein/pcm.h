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

struct PcmBuffer {
	void	*ch[2];			/* buffer for each channel */
	int		w;				/* sample width */
	int		n;				/* number samples allocated */
	int		u;				/* number samples used */
#if 0 /* just pcm data */
	int		skip_start;		/* number samples to ignore at the beginning */
	int		skip_end;		/* number samples to ignore at the end */
#endif
};

typedef struct PcmBuffer PcmBuffer;

typedef struct _PCM_READER_DATA_T
{
	int				count_samples;
	int				pcmbitwidth;
	int				pcmswapbytes;
	int				pcm_is_unsigned_8bit;
	int				pcm_is_ieee_float;
	unsigned int	num_smaples_read;
	FILE*			wave_in;
	PcmBuffer		pcm32;
	PcmBuffer		pcm16;
	PcmBuffer		pcmfloat;
	size_t			in_id3v2_size;
	unsigned char*	in_id3v2_tag;
	void*			info; /* pointer to WAVE_FILE_INFO_T */
} pcm_reader_data;

int initPCM_Reader(pcm_reader_data **pcmhandle);
int setPCM_OpenWaveFile(pcm_reader_data *pcmhandle, const char *file);
int setPCM_WaveFile(pcm_reader_data *pcmhandle, FILE *infp);
int setPCM_FromWaveInfo(pcm_reader_data* pcmhandle, FILE *infp, void* waveinfo);

int readPCM_data(pcm_reader_data* pcmhandle, int toread);
int readPCM_data_int(pcm_reader_data*, int toread);
int readPCM_data_short(pcm_reader_data*, int toread);
int readPCM_data_ieee_float(pcm_reader_data*, int toread);

	
#ifdef __cplusplus
}
#endif

#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "nein/wave.h"
#include "lame/lame.h"


extern char* get_suggested_filename(char *src, char*dst_ext);

static unsigned char vbuf[1152*4*2];
static int num_samples_read = 0;
WAVE_FILE_INFO_T * info = NULL;

int set_data_position(FILE *ifp, WAVE_FILE_INFO_T	*wfinfo);
void lame_encode(FILE *infp, FILE* ofp, WAVE_FILE_INFO_T *wfinfo);
void read_test (FILE *ifp);

int main (int ac, char** av)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (NULL!=(info = getWaveInfoFromFile(av[1]))) {
		long length = 0;
		FILE *fp = fopen (av[1], "rb");
		FILE	*ofp = NULL;
		char	*dst = NULL;
		fseek(fp, 0L, SEEK_END);
		length = ftell(fp);
	//	fclose (fp);
//		fp = NULL;
		printf("\nFile: %s\n", av[1]);
		printf("length: %ld\n", length);
		printWaveInfo(info);
//
//		fp = fopen (av[1], "r");
//		if (!fp){
//			printf ("fopen failed. [%s]\n", av[1]);
//		}

		set_data_position(fp, info);
		read_test(fp);
		dst = get_suggested_filename (av[1], NULL);
		ofp = fopen (dst, "wb");
		lame_encode(fp,ofp,info);

		fclose (ofp);
		fclose (fp);

	}

	return 0;
}

int set_data_position(FILE *ifp, WAVE_FILE_INFO_T	*wfinfo)
{
	long data_offset = wfinfo->datainfo.offset ;
	return fseek(ifp, data_offset, SEEK_SET);
}

void setup_lame_config(lame_t gfp, WAVE_FILE_INFO_T *wfinfo)
{
	int channels = 0;
	int	sampleRate = 0;
	unsigned long samples_per_sec = 0;
	long	data_length = 0;
	int		bits_per_sample = 0;
	unsigned long num_samples = 0;

	channels = (int)getWAVEChannels(wfinfo);
	lame_set_num_channels (gfp, channels);

	samples_per_sec = (unsigned long)getWAVESampleRate (wfinfo);
	lame_set_in_samplerate(gfp, samples_per_sec);
	if (samples_per_sec < 16000)
		lame_set_out_samplerate(gfp, 16000);

	data_length = getWAVEDataLength (wfinfo);
	if (data_length > 0 && data_length%2) data_length += 1;

	bits_per_sample = getWAVEBitsPerSample (wfinfo);

	num_samples=(unsigned long)(data_length/(channels*((bits_per_sample+7)/8)));
	lame_set_num_samples(gfp, num_samples);
}

void
write_id3_tag (lame_t gfp, FILE *outfp)
{
	unsigned char mp3buffer[LAME_MAXMP3BUFFER];
	size_t  id3v2_size;
	int     imp3;
	id3v2_size = lame_get_id3v2_tag (gfp, 0, 0);
	if (id3v2_size > 0)
	{
		unsigned char *id3v2tag = malloc (id3v2_size);
		if (id3v2tag)
		{
			imp3 = lame_get_id3v2_tag (gfp, id3v2tag, id3v2_size);
			(void)fwrite (id3v2tag, 1, imp3, outfp);
			free (id3v2tag);
		}
	}
}


int get_data (lame_t gfp, FILE *ifp, int buf[2][1152])
{
	int num_channels = lame_get_num_channels(gfp);
	int insamp[2*1152];
	int samples_read = 0;
	int framesize;
	int samples_to_read;
	unsigned int remaining, tmp_num_samples;
	int	bytes_per_sample;
	int i;
	int *p;
	unsigned char *inp ;
	int	ifno = fileno(ifp);
	int to_read ;

	samples_to_read = framesize = lame_get_framesize(gfp);
	tmp_num_samples = lame_get_num_samples(gfp);
	printf ("number of samples: %d/%d\n", num_samples_read,tmp_num_samples);

	if (num_samples_read < tmp_num_samples) {
		remaining = tmp_num_samples - num_samples_read;
	} else 
		remaining = 0;
	if (remaining < (unsigned int)framesize && 0 != tmp_num_samples)
		samples_to_read = remaining;

	if (samples_to_read <=0){
		return 0;
	}

	samples_to_read *= num_channels;

	printf ("(%05d     ) bf fread: %ld\n",samples_to_read, ftell(ifp));

	inp = (unsigned char*)insamp;
	to_read = samples_to_read;// * sizeof(int);
	do {
		int n_read = 0;
		//inp = ((unsigned char*)insamp)+(samples_read * sizeof(int));	

		n_read = fread ((void*)inp, sizeof(int), to_read, ifp);
		//n_read = read (ifno, inp, to_read);

		printf ("[%d] Req[%d], %d read, %d to read (offset %ld)\n"
				, n_read ,samples_to_read, samples_read, to_read, ftell(ifp));
		if (n_read <0){
			break;
		}
//		if (ferror(ifp)) {
//			printf ("file stream error. \n");
//			break;
//		}

		if (n_read == 0){
			sleep(1);
		}
		inp += (n_read * num_channels * sizeof(int));
		to_read -= n_read;
		samples_read += n_read;
	}while(to_read>0);

	printf ("(%05d:%05d) af fread: %ld\n"
			, samples_read
			, samples_read * sizeof(int)
			, ftell(ifp));

	if (samples_read < 0) return samples_read;

//	samples_read /= sizeof(int);

	p = insamp + (samples_read); // * num_channels);
	samples_read /= num_channels;

	if (num_channels == 2) {
		for (i = samples_read; --i >=0 ; ){
			buf[1][i] = *--p;
			buf[0][i] = *--p;
		}
	} else if (num_channels == 1) {
		for (i = samples_read; --i >=0 ; ){
			buf[0][i] = *--p;
		}
	}
	num_samples_read += samples_read;
	return samples_read;
}

void lame_encode (FILE *infp, FILE* ofp, WAVE_FILE_INFO_T *wfinfo)
{
	lame_t	gfp = NULL;
	unsigned char	mp3buf[LAME_MAXMP3BUFFER];
	int		Buf[2][1152];
	int		iread, imp3, owrite;

	gfp = lame_init();

	id3tag_init(gfp);
	lame_set_findReplayGain(gfp, 1);

	setup_lame_config(gfp, wfinfo);

	lame_set_quality (gfp, 2);
//	lame_set_write_id3tag_automatic(gfp, 0);
	lame_init_params(gfp);
//	write_id3_tag (gfp, ofp);

	set_data_position(infp, wfinfo);	

	do {
		iread = get_data(gfp, infp, Buf);
		printf ("%d samples were read\n", iread);

		imp3 = lame_encode_buffer_int(
					gfp,Buf[0],Buf[1],iread,mp3buf,sizeof(mp3buf));
		printf ("lame_encode_buffer_int : %d\n", imp3);
		if (imp3>0) {
			owrite = fwrite (mp3buf, 1, imp3, ofp);
			fflush(ofp);
		}

	}while (iread >0);

	imp3 = lame_encode_flush(gfp, mp3buf, sizeof(mp3buf));

	if (imp3>0)
		owrite = fwrite (mp3buf, 1, imp3, ofp);

}

void read_test (FILE *ifp)
{
	int buf[1152*2];
	int nread  =0 ;
	long	lf, af;
	while (!feof(ifp)){
		lf = ftell(ifp);
		printf (" @%ld\n", lf);
		nread = fread (buf, 4, 2304, ifp);
		af = ftell(ifp);
		printf ("%d int's read @%ld (%ld)(%ld)\n", nread, af, af-lf, (af-lf)/4);
	}
}

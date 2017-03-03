/*
 *
    This is neinWav2mp3. 
	A simple application converts wav filee to mp3 file.
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
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h> /* for strerror */
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include "lame/lame.h"

#include "nein/osa.h"
#include "nein/wave.h"
#include "nein/pcm.h"
#include "workqueue.h"
#include "jobitem.h"
#include "getname.h"
#include "nein/debug.h"
#include "version.h"

void showUsage(char *progname){
	printf ("%s ,Version: %s\n", progname, NIL_W2M_VERSION);
	printf ("Usage: %s dir_path \n", progname);
#if defined (__MINGW32__)
	printf ("Example: %s F:\\Music \n", progname);
#else
	printf ("Example: %s /home/nein/wavefiles \n", progname);
#endif
}

/** global variables */
static int bQuit = 0;
static int doneCrawling = 0;
static int nTotal = 0;
static int nFree = 0;
static int nDoing = 0;
static int nDone = 0;;

static nil_task_mgm_t	*taskmanager = NULL;
static workqueue_t		*workQueue = NULL;
static int	numOfCores = 0;

#define LIST_OF_FREE	0
#define LIST_OF_DOING	1
#define LIST_OF_DONE	2

typedef workitem_t* (*nextWork)(workqueue_t *) ;

nextWork nextItem[3] = {
	getNextFreeWork,
	getNextDoingWork,
	getNextDoneWork
};

pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  gCond = PTHREAD_COND_INITIALIZER;;

void broadcastingCond(void);
void broadcastingCond (void)
{
	(void)pthread_cond_broadcast(&gCond);
}

void sighandler(int sno)
{
	if (sno == SIGINT){
		/* broadcasting signal */
		bQuit = 1;
		broadcastingCond();
	}
}

static void	write_id3_tag (lame_t, FILE * );
static void print_item (workitem_t *wit)
{
	jobitem_t *job = NULL;
	pcm_reader_data	*pcmhdl = NULL;
	WAVE_FILE_INFO_T *winfo = NULL;
	job = (jobitem_t*)(wit->priv);
	if (job) 
	{
		printf ("File: %s\n", job->src?job->src:"");
		printf ("Dst:  %s\n", job->dst?job->dst:"");
		printf ("Is this file supported to convert? %s\n",
				job->isSupported?"YES":"NO");
		pcmhdl = (pcm_reader_data*)(job->pcmInfo);
		if (pcmhdl)
			winfo = (WAVE_FILE_INFO_T *)(pcmhdl->info);
		if (winfo)
		{
			//printf ("%20p | %20p | %20p | %20p \n",
			//		wit->priv, wit, wit->list.prev, wit->list.next);
			printWaveInfo(winfo);
		}
	}
}

static void print_list (workqueue_t *wq, int listType, int limit)
{
	workitem_t *wit = NULL;
	
	//printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	wit = nextItem[listType](wq);
	while (wit && limit) {
		print_item(wit);
		wit = nextItem[listType](wq);
		limit--;
	}
}

static void printWorkResult (workqueue_t *wq)
{
	if (wq) 
	{
		getNumbState(wq, &nTotal, &nFree, &nDoing, &nDone);

		wq->curFree = wq->curDoing = wq->curDone = NULL;

		if (nFree>0) 
		{
			printf ("Free working list (%d)\n",nFree);
			print_list (wq, LIST_OF_FREE, nFree);
		}
		if (nDoing>0)
		{
			printf ("Doing working list (%d)\n",nDoing);
			print_list (wq, LIST_OF_DOING, nDoing);
		}
		if (nDone>0)
		{
			printf ("Done working list (%d)\n",nDone);
			print_list (wq, LIST_OF_DONE, nDone);
		}

		wq->curFree = wq->curDoing = wq->curDone = NULL;
	}
}

static int set_signal(void)
{
	struct sighandle_set priHdl;
	int	   rv = 0;

	priHdl.signo = SIGINT;
	priHdl.hndl = sighandler;
	if(0!=(rv = setSigHandler(&priHdl)))
	{
		rv = -errno;
	}

	return rv;
}

static void freeJobs (void *p) 
{
	if (p)
	{
		pcm_reader_data *data = (pcm_reader_data*)p;
		freePCM_Reader (data);
	}
}

static int writeId3Tag (lame_t, FILE* , unsigned char *);
static int setup_lame_config (lame_t gfp, WAVE_FILE_INFO_T *wfinfo);

/**
  @param  path  wether this path is a diretory or not.
  @param  ppdir pointer to pointer to DIR, 
                if this value is null, a pointer to DIR will be closed.
  returns 0 if path is a direcotry
		other -errnor
*/
   
static int isDirectory(const char *path, DIR** ppdir)
{
	DIR	*pdir = NULL;
	if (path == NULL) {
		return -EINVAL;
	}
	if (!(pdir = opendir(path))) {
		NIL_ERROR("opendir returns %d:%s\n",errno, strerror(errno));
		return -errno;
	}
	if (ppdir != NULL)
		*ppdir = pdir;
	else 
		closedir(pdir);
	return 0;
}

#define DEF_NSAMPLE	1152
#define DEF_BUF_SIZE	(2*DEF_NSAMPLE)

void * nilWorks (void *arg) 
{
	int bForcedQuit = 0;
	workqueue_t		*wqueue = NULL;
	workitem_t		*work = NULL;
	jobitem_t		*job = NULL;
	lame_t			lame_gfp = NULL;
	int				is_lame_initialized = 0;
	unsigned char	*mp3buffer = NULL;
	PcmBuffer		pcm16 = {0, };
	PcmBuffer		pcm32 = {0, };
	PcmBuffer		pcmfloat = {0, };
	PcmBuffer		pcmdouble = {0, };

	initPcmBuffer (&pcm16, DEF_NSAMPLE, sizeof(int16_t));
	initPcmBuffer (&pcm32, DEF_NSAMPLE, sizeof(int32_t));
	initPcmBuffer (&pcmfloat, DEF_NSAMPLE, sizeof(float));
	initPcmBuffer (&pcmdouble, DEF_NSAMPLE, sizeof(double));

	while (!bQuit) 
	{
		pthread_mutex_lock (&gMutex);
		pthread_cond_wait (&gCond, &gMutex);
		pthread_mutex_unlock (&gMutex);
		if (!mp3buffer) 
		{
			mp3buffer = (unsigned char *)malloc (LAME_MAXMP3BUFFER);
			if (!mp3buffer)
			{
				NIL_ERROR ("mp3buffer: allocation failed.\n");
				break;
			}
			memset (mp3buffer, 0x00, LAME_MAXMP3BUFFER);
		}

		do
		{
			if (bQuit) {
				break;
			}
			work = popWorkFromFree(workQueue);
			if (work) 
			{
				WAVE_FILE_INFO_T *wfinfo = NULL;
				pcm_reader_data	 *pcmhdl = NULL;
				addItem2Doing(workQueue, work);
				job = (jobitem_t*)(work->priv);
				NIL_DEBUG ("[%p] working jobs : %s\n", pthread_self(),job->src);
				set_state (job, WORK_DOING);
				job->isSupported =0;
				/* check pcm header */
				/* if this file is pcm file and supported in this program 
				   then working */
				if (NULL!=(wfinfo = getWaveInfoFromFile (job->src))) 
				{
					FILE *outfp = NULL;
					//int		channels = 0;
					//int		samples_per_sec = 0;
					//unsigned long	num_samples = 0;
					long	data_length = 0;
					int		bits_per_sample = 0;
					uint16_t	formatTag = 0;

					initPCM_Reader (&pcmhdl, EXT_PCM_BUFFER);
					resetPcmBuffer (&pcm16);
					resetPcmBuffer (&pcm32);
					resetPcmBuffer (&pcmfloat);
					resetPcmBuffer (&pcmdouble);

					setPCM_PcmBuffer (pcmhdl, &pcm16, &pcm32, &pcmfloat, &pcmdouble);

					setPcmData (job, (void*)pcmhdl);
					setPCM_FromWaveInfo (pcmhdl, NULL, (void*)wfinfo);

					job->isSupported = isSupportedWAVEFile(wfinfo);

					if (job->isSupported)
					{
						int nread = 0;
						int imp3 = 0;
						int nwrite = 0;
						int rc = 0;

						formatTag = getWAVEFormatTag (wfinfo);
						job->dst = get_suggested_filename (job->src, NULL);
						NIL_DEBUG ("Src[%s] --> Dst[%s]\n", job->src, job->dst);
						outfp = fopen (job->dst, "w+b");
						setSupportedFlag (job, SUPPORTED_FILE);
						setPCM_file_position (pcmhdl);
						bits_per_sample = getWAVEBitsPerSample (wfinfo);
						if (!lame_gfp)
						{
							lame_gfp = lame_init();
							if (!lame_gfp)
							{
								NIL_ERROR("lame_init: error.\n");
								break;
							}
						}

						id3tag_init(lame_gfp);
						lame_set_findReplayGain(lame_gfp, 1);

						if (-1 == setup_lame_config (lame_gfp, wfinfo))
						{
							job->isSupported = 0;
							goto routine_not_supported;
						}

						lame_set_quality (lame_gfp, 2);
						NIL_DEBUG ("MPEG_mode: %d\n", lame_get_mode(lame_gfp));

						lame_set_write_id3tag_automatic(lame_gfp, 0);

						if (is_lame_initialized)
						{
							rc = lame_init_bitstream (lame_gfp);
							if (rc < 0)
							{
								NIL_ERROR("lame_init_bitstream failed.\n");
								job->isSupported = 0;
								goto routine_not_supported;
							}
						}
						else
						{
							rc = lame_init_params(lame_gfp);
							if (rc <0)
							{
								NIL_ERROR("lame_init_params failed.\n");
								job->isSupported = 0;
								goto routine_not_supported;
							}
							is_lame_initialized = 1;
						}
#if !defined(NIL_DEBUG_OFF)
						lame_print_config (lame_gfp);
#endif
						/* write id3 tag */
						write_id3_tag (lame_gfp, outfp);

						while (!bQuit)
						{
							int	to_read_samples = 0;
							int	remainSamples = pcmhdl->numOfSamples - pcmhdl->num_samples_read;
							int	frameSize = 0;
							to_read_samples = frameSize = lame_get_framesize(lame_gfp);
							if (remainSamples < 0) remainSamples = 0;

							if (remainSamples < frameSize 
									&& 0!= pcmhdl->numOfSamples) 
							{
								to_read_samples = remainSamples;
							}

							nread = readPCM_data(pcmhdl, to_read_samples);
							if (nread <=0) {
								break;
							}
							if (WAVE_FORMAT_PCM == formatTag)
							{
								if (bits_per_sample <= 16)
								{
									short *ch[2];
									ch[0] = (short*)(pcmhdl->pcm16->ch[0]);
									ch[1] = (short*)(pcmhdl->pcm16->ch[1]);
									imp3 = lame_encode_buffer(lame_gfp,ch[0],ch[1],nread, mp3buffer, LAME_MAXMP3BUFFER);
								}
								else // if (bits_per_sample > 16: 24bit, 32bit)
								{
									int *ch[2];
									ch[0] = (int*)(pcmhdl->pcm32->ch[0]);
									ch[1] = (int*)(pcmhdl->pcm32->ch[1]);
									imp3 = lame_encode_buffer_int (lame_gfp,ch[0],ch[1],nread, mp3buffer, LAME_MAXMP3BUFFER);
								}
							}
							else if (WAVE_FORMAT_IEEE_FLOAT == formatTag)
							{
								if (bits_per_sample == 32)
								{
									float *ch[2];
									ch[0] = (float*)(pcmhdl->pcmfloat->ch[0]);
									ch[1] = (float*)(pcmhdl->pcmfloat->ch[1]);
									imp3 = lame_encode_buffer_ieee_float (lame_gfp,ch[0],ch[1],nread, mp3buffer, LAME_MAXMP3BUFFER );
								}
								else if (bits_per_sample == 64)
								{
									double *ch[2];
									ch[0] = (double*)(pcmhdl->pcmdouble->ch[0]);
									ch[1] = (double*)(pcmhdl->pcmdouble->ch[1]);
									imp3 = lame_encode_buffer_ieee_double (lame_gfp,ch[0],ch[1],nread, mp3buffer, LAME_MAXMP3BUFFER );
								}
								else 
								{
									job->isSupported = 0;
									goto routine_not_supported;
								}
							}
							else
							{
								job->isSupported = 0;
								goto routine_not_supported;
							}

							pcmhdl->num_samples_read += nread;

							if (imp3<0) 
							{
								job->isSupported = 0;
								goto routine_not_supported;
							}
							/* write encoded buffer to output file */
							nwrite = fwrite (mp3buffer, 1, imp3, outfp);
							if (nwrite != imp3)
							{
								NIL_ERROR("fwrite %d %d %d\n"
										,imp3, nwrite, imp3-nwrite);
							}
						}
						/* lame_encode_flush */
						imp3 = lame_encode_flush(lame_gfp, mp3buffer, LAME_MAXMP3BUFFER);
						if (imp3>0) 
						{
							/* write flushing data to output file */
							(void)fwrite (mp3buffer, 1, imp3, outfp);
						}
						fflush (outfp);
						/* close output file */
routine_not_supported:
						if (outfp)
						{
							fclose (outfp);
							outfp = NULL;
						}
						if (!job->isSupported)
						{
							unlink (job->dst);
						}
					}
					if (lame_gfp) {
						lame_close(lame_gfp);
						lame_gfp = NULL;
						is_lame_initialized = 0;
					}
				}

				if(1==bQuit)
					set_state (job,WORK_ABORTED);
				else
					set_state (job, WORK_DONE);
				addItem2Done(workQueue, work);
			}
		} while (work);
	}
	freePcmBuffer (&pcm16);
	freePcmBuffer (&pcm32);
	freePcmBuffer (&pcmfloat);
	freePcmBuffer (&pcmdouble);
	if (lame_gfp) lame_close (lame_gfp);
	return (void*)0;
}

static int createTasks (int numCores)
{
	nil_task_t	*task = NULL;
	int			rc = 0;
#if !defined(NIL_ONE_THREAD)
	int			limitTasks = numCores * 2;
#else
	int			limitTasks = 1;
#endif
	for ( ; limitTasks > 0 ; limitTasks--) {
		initTask (&task);
		task->onCore = (limitTasks%numCores);
		task->work_run = nilWorks;
		if (0>(rc=createTask (task))){
			NIL_ERROR ("createTask returns %d\n", rc);
		}
		addTask2TaskMgm (taskmanager, task);
	}
	return 0;
}

void gatheringData (const char *fpath)
{
	jobitem_t *job = NULL;
	workitem_t *wi = NULL;
	if (!fpath) return ;
	NIL_DEBUG (">>> %s\n", fpath);
	job = alloc_jobitem ((char*)fpath);
	if (!job) {
		return;
	}
	wi = allocWorkItem ((void*)job, sizeof(jobitem_t));
	if (!wi) {
		free_jobitem (job);
		return ;
	}
	if (0!=addNewItem (workQueue, wi)){
		free(wi);
		free_jobitem (job);
		return;
	}
#if 0
	if (0==getNumbState(workQueue, &nTotal, &nFree, &nDoing, &nDone))
	{
		if (nTotal%numOfCores)
			return ;
	}
#endif	
	pthread_mutex_lock(&gMutex);
	pthread_cond_signal(&gCond);
	pthread_mutex_unlock(&gMutex);
	return ;
}

static void completedCallBack(void)
{
	/* set timer */
	if (doneCrawling 
		&& 0==getNumbState(workQueue, &nTotal, &nFree, &nDoing, &nDone)){
		if (nTotal == nDone) {
			bQuit = 2;
			broadcastingCond();
		}
	}
	return;
}

int main (int ac, char **av)
{
	char *lookingdir = NULL;
	if (ac < 2){
		showUsage(*av);
		return 1;
	}
#if defined (__MINGW32__)
	setvbuf (stdout, NULL, _IONBF, 0);
#endif
	set_signal();

	/* check whether  argument is a directory or not? */
	if (0 != isDirectory((const char*)*(av+1), NULL)){
		NIL_ERROR ("%s is not a directory.\n", *(av+1));
		return 2;
	}

	init_wq(&workQueue);
	setCompleteFn (workQueue, completedCallBack);
	set_Freefunction (freeJobs);
	initTaskManager (&taskmanager);
	getNumOfCores(&numOfCores);
	createTasks(numOfCores);

	lookingdir = strdup(av[1]); //(char*)*(av+1);
	//lookingdir = (char*)*(av+1);
	if (!lookingdir)
	{
		bQuit = 1;
		broadcastingCond();
	}
	else
	{
		NIL_DEBUG("looking for files in %s\n", lookingdir);
		walkThDir (lookingdir, gatheringData);
		doneCrawling = 1;
		if (0==getNumbState(workQueue, &nTotal, &nFree, &nDoing, &nDone))
		{
			if (nTotal == 0)
			{
				bQuit = 1;
				broadcastingCond();
			}
			completedCallBack();
		}

	}

	waitAllTasks (taskmanager);

	printWorkResult (workQueue);

	destroy_wq (&workQueue);
	destroy_TaskManager(&taskmanager);
	free(lookingdir);
	return 0;
}

int setup_lame_config (lame_t gfp, WAVE_FILE_INFO_T *wfinfo)
{
	int channels = 0;
	int sampleRate =  0;
	unsigned long num_samples = 0;
	long	data_length = 0;
	int		bits_per_sample = 0;
	unsigned long samples_per_sec = 0;
	//unsigned long nSamples = 0;

	/* set number of channels */
	channels = (int)getWAVEChannels(wfinfo);
	if (-1 == lame_set_num_channels (gfp, channels))
	{
		NIL_ERROR ("lame_set_num_channels return -1, (%d)\n", channels);
		return -1;
	}
	NIL_DEBUG ("channels %d, %d\n", channels, lame_get_num_channels(gfp));

	/* set sample rate */
	samples_per_sec = (unsigned long)getWAVESampleRate (wfinfo);
	if (-1 == lame_set_in_samplerate (gfp, samples_per_sec))
	{
		NIL_ERROR ("lame_set_in_samplerate -1 (%lu)\n", samples_per_sec);
		return -1;
	}
	NIL_DEBUG ("samples/sec %lu, %d\n", samples_per_sec
			, lame_get_in_samplerate(gfp));
	if (samples_per_sec < 16000)
	{
		(void)lame_set_out_samplerate(gfp, 16000);
	}
#if 0
	else 
	{
		int rc = 0;
		printf ("out samplerate: %d\n", lame_get_out_samplerate(gfp));
		rc = lame_set_out_samplerate(gfp, samples_per_sec);
		printf ("[%d]out samplerate: %d\n",rc, lame_get_out_samplerate(gfp));
	}
#endif
	/* set number of samples */
	data_length = getWAVEDataLength (wfinfo);
	if (data_length > 0 && data_length%2)
	{
		data_length += 1;
	}
	NIL_DEBUG ("Data length: %ld\n", data_length);
	bits_per_sample = getWAVEBitsPerSample (wfinfo);
	num_samples = (unsigned long)(data_length/(channels*((bits_per_sample + 7)/8)));
	lame_set_num_samples (gfp, num_samples);

	return 0;
}

void
write_id3_tag (lame_t gfp, FILE *outfp)
{
	unsigned char mp3buffer[LAME_MAXMP3BUFFER];
	size_t	id3v2_size;
	int		imp3;
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

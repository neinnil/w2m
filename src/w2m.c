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

#include "nein/osa.h"
#include "nein/wave.h"
#include "workqueue.h"
#include "jobitem.h"

#define NIL_DEBUG(fmt, ...) do {			\
	fprintf(stderr, fmt##__VA_ARGS__);		\
}while(0)

void showUsage(char *progname){
	printf ("Usage: %s dir_path \n", progname);
	printf ("Example: %s F:\\Music \n", progname);
	printf ("Example: %s /home/nein/wprognamees \n", progname);
}

/** global variables */
static int bQuit = 0;
static int doneCrawing = 0;
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

void print_item (workitem_t *wit)
{
	jobitem_t *job = NULL;
	WAVE_FILE_INFO_T *winfo = NULL;
	job = (jobitem_t*)(wit->priv);
	if (job) 
	{
		winfo = (WAVE_FILE_INFO_T *)(job->pcmInfo);
		if (winfo)
		{
			printf ("%20p | %20p | %20p | %20p \n",
					wit->priv, wit, wit->list.prev, wit->list.next);
			printWaveInfo(winfo);
			printf ("Is this file supported to convert? %s",
					job->isSupported?"YES":"NO");
		}
	}
}

static void print_list (workqueue_t *wq, int listType, int limit)
{
	workitem_t *wit = NULL;
	
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
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

		printf ("Free working list\n");
		print_list (wq, LIST_OF_FREE, nFree);
		printf ("Doing working list\n");
		print_list (wq, LIST_OF_DOING, nDoing);
		printf ("Done working list\n");
		print_list (wq, LIST_OF_DONE, nDone);

		wq->curFree = wq->curDoing = wq->curDone = NULL;
	}
}

static int set_signal(void)
{
	struct sighandle_set priHdl;
	priHdl.signo = SIGINT;
	priHdl.hndl = sighandler;
	int	   rv = 0;;
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
		WAVE_FILE_INFO_T *info = (WAVE_FILE_INFO_T*)p;
		freeWaveInfo (info);
	}
}

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
		printf("opendir returns %d:%s\n",errno, strerror(errno));
		return -errno;
	}
	if (ppdir != NULL)
		*ppdir = pdir;
	else 
		closedir(pdir);
	return 0;
}

void * nilWorks (void *arg) 
{
	int bForcedQuit = 0;
	workqueue_t	*wqueue = NULL;
	workitem_t *work = NULL;
	jobitem_t  *job = NULL;
	while (!bQuit) {
		pthread_mutex_lock (&gMutex);
		pthread_cond_wait (&gCond, &gMutex);
		pthread_mutex_unlock (&gMutex);
		do {
			if (bQuit) {
				break;
			}
			work = popWorkFromFree(workQueue);
			if (work) {
				addItem2Doing(workQueue, work);
				job = (jobitem_t*)(work->priv);
				printf ("working jobs : %s\n", job->src);
				set_state (job, WORK_DOING);
				/* check pcm header */
				/* if this file is pcm file and supported in this program 
				   then working */
				{
					WAVE_FILE_INFO_T *wfinfo = NULL;
					if (NULL!=(wfinfo = getWaveInfoFromFile (job->src))) {
						job->isSupported = isSupportedWAVEFile(wfinfo);
						setPcmData (job, (void*)wfinfo);
					}
				}
				setSupportedFlag (job, SUPPORTED_FILE);
				set_state (job, WORK_DONE);
				addItem2Done(workQueue, work);
			}
		} while (work);
	}
	return (void*)0;
}

static int createTasks (int numCores)
{
	nil_task_t	*task = NULL;
	int			rc = 0;
	int			limitTasks = numCores * 2;
	for ( ; limitTasks > 0 ; limitTasks--) {
		initTask (&task);
		task->onCore = (limitTasks%numCores);
		task->work_run = nilWorks;
		addTask2TaskMgm (taskmanager, task);
		if (0>(rc=createTask (task))){
			printf ("createTask returns %d\n", rc);
		}
	}
	return 0;
}

void gatheringData (const char *fpath)
{
	jobitem_t *job = NULL;
	workitem_t *wi = NULL;
	if (!fpath) return ;
	printf (">>> %s\n", fpath);
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
	pthread_mutex_lock(&gMutex);
	pthread_cond_signal(&gCond);
	pthread_mutex_unlock(&gMutex);
	return ;
}

static void completedCallBack(void)
{
	/* set timer */
	if (doneCrawing 
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
		printf ("%s is not a directory.\n", *(av+1));
		return 2;
	}

	init_wq(&workQueue);
	setCompleteFn (workQueue, completedCallBack);
	set_Freefunction (freeJobs);
	printf ("--af initializing workqueue. \n");
	initTaskManager (&taskmanager);
	printf ("--af initializing task manager. \n");
	getNumOfCores(&numOfCores);
	printf ("--af %d cores. \n", numOfCores);
	createTasks (numOfCores);
	printf ("--af creating task.. \n");

	lookingdir = (char*)*(av+1);
	printf ("looking for files in %s\n", lookingdir);
	walkThDir (lookingdir, gatheringData);
	doneCrawing = 1;
	waitAllTasks (taskmanager);

	printWorkResult (workQueue);

	destroy_wq (&workQueue);
	destroy_TaskManager(&taskmanager);
	return 0;
}

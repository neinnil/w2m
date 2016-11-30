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

static nil_task_mgm_t	*taskmanager = NULL;
static workqueue_t		*workQueue = NULL;
static int	numOfCores = 0;

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
					WAVE_FILE_INFO_T *info = NULL;
					if (NULL!=(info = getWaveInfoFromFile (job->src))) {
						printWaveInfo(info);
						FREEWAVEFILEINFO(info);
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
	}
	pthread_mutex_lock(&gMutex);
	pthread_cond_signal(&gCond);
	pthread_mutex_unlock(&gMutex);
	return ;
}

static void completedCallBack(void)
{
	/* set timer */
	int nTotal, nFree, nDoing, nDone;
	if (0==getNumbState(workQueue, &nTotal, &nFree, &nDoing, &nDone)){
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

	waitAllTasks (taskmanager);

	destroy_wq (&workQueue);
	destroy_TaskManager(&taskmanager);
	return 0;
}

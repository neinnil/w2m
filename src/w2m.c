#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h> /* for strerror */
#include <errno.h>
#include <pthread.h>

#include "nein/osa.h"
#include "workqueue.h"
#include "jobitem.h"

#define NIL_DEBUG(fmt, ...) do {
	fprintf(stderr, fmt##__VA_ARGS__);
}while(0)

void showUsage(char *progname){
	printf ("Usage: %s dir_path \n", progname);
	printf ("Example: %s F:\\Music \n", progname);
	printf ("Example: %s /home/nein/wprognamees \n", progname);
}

struct wQ {
	struct	  list;
	char	  *src_path;
	char	  *dst_path;
	void	  *pcmData;
	pthread_t taker;
	int		  state; /* 0: not owned, 1: owned, 2: done, 16: not supported */
};


/** global variables */
static int bQuit = 0;

static nil_task_mgm_t	*taskmanager = NULL;
static workqueue_t		*workQueue = NULL;
static int	numOfCores = 0;

pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  gCond = PTHEAD_COND_INITITIALIZER;;

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
	if(0!=(rv = setSigHandler(priHdl)))
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
		if (bQuit) {
			break;
		}
		wqueue = popWorkFromFree(workQueue);
		if (wqueue) {
			addItem2Doing(workQueue, work);
			job = (jobitem_t*)wqueue->priv;
			set_state (job, WORK_DOING);
			/* check pcm header */
			/* if this file is pcm file and supported in this program 
			   then working */
			setSupportedFlag (job, SUPPORTED_FILE);
			set_state (job, WORK_DONE);
			addItem2Done(workQueue, work);
		}
	}
	return (void*)0;
}

static int createTasks (int numCores)
{
	nil_task_t	*task = NULL;
	int			limitTasks = numCores * 2;
	for ( ; limitTasks > 0 ; limitTasks--) {
		initTask (&task);
		task->onCore = (limitTasks%numCores);
		task->work_run = nilWorks;
		addTask2TaskMgm (taskmanager, task);
	}
}

void gatheringData (const char *fpath)
{
	jobitem_t *job = NULL;
	workitem_t *wi = NULL;
	if (!dpath) return ;
	job = alloc_jobitem ((char*)fpath);
	if (!job) {
		return;
	}
	wi = allocWorkItem ((void*)job, sizeof(jobitem_t));
	if (!wi) {
		free_jobitem (job);
		return ;
	}
	pthread_mutex_lock(&gMutex);
	if (0!=addNewItem (workQueue, wi)){
		free(wi);
		free_jobitem (job);
		pthread_mutex_unlock(&gMutex);
	}
	pthread_cond_signal(&gCond);
	pthread_mutex_unlock(&gMutex);
	return ;
}

int main (int ac, char **av)
{
	char *lookingdir = NULL;
	if (ac < 2){
		showUsage(*av);
		return 1;
	}

	set_signal();

	/* check whether  argument is a directory or not? */
	if (0 != isDirectory((const char*)*(av+1), NULL)){
		return 2;
	}

	init_wq(&workQueue);
	initTaskManager (&taskmanager);
	getNumOfCores(&numOfCores);
	createTasks (numOfCores);

	lookingdir = (char*)*(av+1);
	walkThDir (lookingdir, gatheringData);

	waitAllTasks (taskmanager);

	destroy_wq (&workQueue);
	destroyTask(taskmanager);
	return 0;
}

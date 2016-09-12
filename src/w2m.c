#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h> /* for strerror */
#include <errno.h>

#define NIL_DEBUG(fmt,arg...) do {
	fprintf(stderr, fmt##arg);
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

extern void broadcastingCond(void);

/** global variables */
static int bQuit = 0;

void sighandler(int sno)
{
	if (sno == SIGINT){
		/* broadcasting signal */
		bQuit = 1;
		broadcastingCond();
	}
}

int setSigHandler(void)
{
	struct sigaction sa;
	int	   rv = 0;;
	memset (&sa, 0x00, sizeof(struct sigaction));
	sa.sa_handler = sighandler;
	if(0!=(rv = sigaction (SIGINT, &sa, NULL)))
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

struct _nil_task {
	pthread_t tid;
	pthread_attr_t *attr;
#ifdef __USE_XOPEN2K
	pthread_barrier_t *barrier;
#endif
	int		sched_priority;
	int		sched_policy;
	int		onCore;
	void	*private;
	void*	(*work_run)(void *);
};

typedef struct _nil_task  nil_task_t;

static void *_task_work(void *taskarg);
static int creatTask(nil_task_t *taskarg);
static int inline _initAttr(pthread_attr_t **attr){
	if (NULL != attr) {
		if (NULL == *attr){
			pthread_attr_t *tattr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
			if (NULL!=tattr) {
				pthread_attr_init(tattr);
				*attr = tattr;
				return 0;
			}
			return -ENOMEM;
		}
		return 0;
	}
	return -EINVAL;
}

int creatTask(nil_task_t *taskarg)
{
	int ret = 0;
	pthread_t tid = (pthread_t)0;
	pthread_attr_t	*attr = NULL;
	pthread_barrier_t *barrier = NULL;
	
	if (NULL == taskarg){
		return -EINVAL;
	}
	if (taskarg->attr)
		attr = taskarg->attr;
#ifdef __USE_XOPEN2K
	if (taskarg->barrier)
		barrier = taskarg->barrier;
	if (barrier) 
		pthread_barrier_wait(barrier);
#endif
	if (-1 !=taskarg->sched_policy) {
		if (-1 !=taskarg->sched_priority){
			if (NULL==attr){
				attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
				if (NULL != attr) {
					if(0==pthread_attr_init(attr)) {
					}
				}
			}
		}
	}
#if !defined(__MINGW__) && defined(__LINUX__)
	if ( -1 != taskarg->onCore ) {
		if (NULL==attr) {
			attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
			if (NULL != attr) {
				if(0==pthread_attr_init(attr)) {
				}
			}
		}
	}
#endif

	ret = pthread_create(&tid, attr, _task_work, (void*)taskarg);
	if (!ret)
		taskarg->tid = tid;
	return ret;
}


int main (int ac, char **av)
{
	DIR	*pdir = NULL;
	if (ac < 2){
		showUsage(*av);
		return 1;
	}

	/* check whether  argument is a directory or not? */
	if (0 != isDirectory((const char*)*(av+1), &pdir)){
		return 2;
	}
	closedir(pdir);
	return 0;
}

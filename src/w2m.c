#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h> /* for strerror */
#include <errno.h>
#include <pthread.h>

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

int main (int ac, char **av)
{
	DIR	*pdir = NULL;
	if (ac < 2){
		showUsage(*av);
		return 1;
	}

	set_signal();

	/* check whether  argument is a directory or not? */
	if (0 != isDirectory((const char*)*(av+1), &pdir)){
		return 2;
	}
	closedir(pdir);
	return 0;
}

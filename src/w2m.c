#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h> /* for strerror */
#include <errno.h>


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
int bQuit = 0;



void sighandler(int sno)
{
	if (sno == SIGINT){
		/* broadcasting signal */
		bQuit = 1;
		broadcastingCond();
	}
}

int setSigHandler(vodi)
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

/*
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
	*ppdir = pdir;
	return 0;
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

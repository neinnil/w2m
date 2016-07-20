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

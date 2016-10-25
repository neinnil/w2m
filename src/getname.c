#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

static char *tmp_name = "TEST FILE";
static char *dfl_ext = "mp3";

int existFile(char *filename)
{
	FILE *fp = NULL;
	if (!filename){
		printf("[%s] Invalid argument.\n", __FUNCTION__);
		return 0;
	}
	fp = fopen (filename, "r");
	if (fp) {
		fclose (fp);
		return 1;
	}
	return 0;
}
		
static char * get_extension(char *filename)
{
	char *p = NULL;
	if (!filename) {
		printf("[%s] Invalid argument.\n", __FUNCTION__);
		return NULL;
	}
	p = strrchr(filename, '.');
	return p;
}

char * get_suggested_filename (char *src, char *dst_ext)
{
	char *tmp = NULL;
	char *base = NULL;
	char *dir_name = NULL;
	char *ext = NULL;
	int		len = FILENAME_MAX;
	if (!src) {
		printf("[%s] Invalid argument.\n", __FUNCTION__);
		return tmp_name;
	}
	if (!dst_ext)
		dst_ext = dfl_ext;
	base = basename(src);
	dir_name = dirname (src);
	ext = get_extension(src);
	printf ("Dirname: %s\n", dir_name);
	printf ("Basename: %s\n", base);
	printf ("Extension: %s\n", ext?ext:"(nil)");
	printf ("Remove extenstion [%s] --> ", base);
	tmp = strrchr(base, '.');
	if (tmp) {
		*tmp = 0x00;
		printf ("[%s]: changed\n", base);
	} else {
		printf ("[%s]: Not changed.\n", base);
	}
	tmp = (char *)malloc(FILENAME_MAX);
	if (tmp) {
		int i = 0;
		snprintf (tmp,FILENAME_MAX,"%s/%s.%s",dir_name, base, dst_ext);
		printf ("Temporary Filename: %s\n", tmp);
		while (existFile(tmp)){
			snprintf (tmp,len,"%s/%s_%d.%s",dir_name, base, i, dst_ext);
			printf ("Temporary Filename: %s\n", tmp);
			i++;
		}
	} else {
		tmp = tmp_name;
	}
	return tmp;
}
#if 0
int main (int ac, char** av)
{
	char *dst = NULL;
	if (ac < 2) {
		printf("Usage: %s filename\n", av[0]);
		return 1;
	}
	dst = get_suggested_filename(av[1]);
	printf ("Source file name: %s\n", av[1]);
	printf ("Suggested file name: %s\n", dst);
	return 0;
}
#endif

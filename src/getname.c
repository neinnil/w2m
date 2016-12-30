/*
    getname.c
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
#include <string.h>
#include <libgen.h>
#include "nein/debug.h"

#include "getname.h"

#define SEPARATOR "/"

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
	char	*tmp = NULL;
	char	*base = NULL;
	char	*dir_name = NULL;
#ifdef __APPLE__
	char	bname[FILENAME_MAX];
	char	dname[FILENAME_MAX];
#else
	char	saved[FILENAME_MAX];
	int		copylen;
#endif
	char	*ext = NULL;
	int		len = FILENAME_MAX;
	if (!src) {
		printf("[%s] Invalid argument.\n", __FUNCTION__);
		return tmp_name;
	}
	if (!dst_ext)
		dst_ext = dfl_ext;
#ifdef __APPLE__
	base = basename_r(src, bname);
	dir_name = dirname_r (src, dname);
#else
	copylen = strlen(src);
	if (copylen >= FILENAME_MAX) 
		copylen = FILENAME_MAX-1;
	strncpy (saved, src, copylen);
	saved[copylen] = '\0';
	base = basename(saved);
	dir_name = dirname (saved);
#endif
	ext = get_extension(src);
	NIL_DEBUG("Dirname: %s\n", dir_name);
	NIL_DEBUG("Basename: %s\n", base);
	NIL_DEBUG("Extension: %s\n", ext?ext:"(nil)");
	NIL_DEBUG("Remove extenstion [%s] --> ", base);
	tmp = strrchr(base, '.');
	if (tmp) {
		*tmp = 0x00;
		NIL_DEBUG("[%s]: changed\n", base);
	} else {
		NIL_DEBUG("[%s]: Not changed.\n", base);
	}
	tmp = (char *)malloc(FILENAME_MAX);
	if (tmp) {
		int i = 0;
		snprintf (tmp,FILENAME_MAX,"%s/%s.%s",dir_name, base, dst_ext);
		NIL_DEBUG ("Temporary Filename: %s\n", tmp);
		while (existFile(tmp)){
			snprintf (tmp,len,"%s/%s_%d.%s",dir_name, base, i, dst_ext);
			NIL_DEBUG ("Temporary Filename: %s\n", tmp);
			i++;
		}
	} else {
		tmp = tmp_name;
	}
#if 0
	{
		char *p = dir_name;
		p += strlen(dir_name);
		printf ("%p(p+strlen) %p(base) \n", p, base);
		if ( base == p+1 ){
			*p = '/';
		}
	}
#endif
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

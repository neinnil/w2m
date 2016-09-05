/*
    This is neinWav2mp3. A simple application converts wav filee to mp3 file.
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
#include "nein_osa.h"

stataic int sysCore = 0;

int setSigHandler(sighandle_set *sigset)
{
	struct sigaction sa;
	sa.sa_handler = (void (*)(int))(sigset->hndl);
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction (sigset->signo, &sa, NULL);
	return 0;
}

int walkThDir(const char *dpath, void(*doing)(const char *fpath))
{
	return 0;
}

pthread_attr_t *setCore (pthread_attr_t *iattr, int onCore)
{
	pthread_attr_t *pattr = NULL;
#if defined(_LINUX_)
	cpu_set_t	cpuset;

	CPU_ZERO(&cpuset);
	if (onCore >= sysCore){
		printf("input value is over sysCore. addjust to %d\n", onCore%sysCore);
		onCore %= sysCore;
	}
	CPU_SET(onCore, &cpuset);
	if (NULL == iattr) {
		if(!(pattr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t)))) 
		{
			printf("Couldn't allocate for pthread_attr_t\n");
			return NULL;
		}
		pthread_attr_init(pattr);
	} else {
		pattr = iattr;
	}
	if (0 != pthread_att_setaffinity_np(&pattr, sizeof(cpu_set_t), &cpuset)){
		if (iattr & pattr){
			printf("Could not setaffinity... \n");
			pthread_attr_destroy(pattr);
			pattr = NULL;
		}
	}
#else if defined(_WIN32_) && defined(PTHREAD_W32)
#else
#endif
	return pattr;
}

int creatTaskOnCore(void *tid,void*(*run)(void *),void* arg,int onCore)
{
	int ret = 0;
	pthread_t thid;
	pthread_attr_t *attr = NULL;

	attr = setCore(NULL, onCore);
	ret = pthread_create(&thid,attr,run, arg);

	return ret;
}

int getNumOfCores(int *core)
{
	int nCore = 1;
#if defined(__LINUX__) || defined (__linux__) || defined(__APPLE__)
	nCore = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined (__MINGW32__) 
#if defined (PTW32_VERSION)
	nCore = pthread_num_processors_np();
#endif
#endif
	if(nCore < 1) nCore = 1;
	*core = sysCore = nCore;
	return nCore;
}

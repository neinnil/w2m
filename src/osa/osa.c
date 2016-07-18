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

int creatTaskOnCore(void **tid,void*(*run)(void *),void* arg,int onCore)
{
	return 0;
}

int getNumOfCores(int *core)
{
	int nCore = 1;
#ifdef _LINUX_ 
	nCore = sysconf(_SC_NPROCESSORS_ONLN);
#else
#endif
	*core = nCore;
	return nCore;
}

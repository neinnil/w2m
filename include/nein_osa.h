/**
 *  This is neinWav2mp3. A simple application converts wav filee to mp3 file.
 *  Copyright (C) 2016  Park, Sangjun
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/
#ifndef _NEIN_OSA_H
#define _NEIN_OSA_H

#ifdef __cplusplus
extern "C" {
#endif

struct sighandle_set {
	int signo;
	void (*hndl)(int);
} ; 
typedef struct sighandle_set sighandle_set; 

extern int setSigHandler(sighandle_set *);

extern int walkThDir(const char *dpath, void(*doing)(const char *fpath));
extern int creatTaskOnCore(void **tid,void*(*run)(void *),void* arg,int core);

extern int getNumOfCores(int *core);


#ifdef __cplusplus
}
#endif

#endif

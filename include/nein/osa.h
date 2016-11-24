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

#include "nein/list.h"

#ifdef __cplusplus
extern "C" {
#endif

	/* signal handler setting. */
struct sighandle_set {
	int signo;
	void (*hndl)(int);
} ; 
typedef struct sighandle_set sighandle_set; 

extern int setSigHandler(sighandle_set *);

extern int walkThDir(char *dpath, void(*doing)(const char *fpath));

/* releated to thread */
struct _nil_task {
	struct nlist ln;
	pthread_t tid;
	pthread_attr_t *attr;
#if defined (__USE_XOPEN2K) || defined (__MINGW32__)
	pthread_barrier_t *barrier; /* refer to barrier pointer */
#endif
	int		sched_priority;
	int		sched_policy;
	int		onCore;
	void	*private;
	void*	(*work_run)(void *);
};

#define RUN_ON_LINUX		1
#define RUN_ON_WIN_MINGW	2
#define RUN_ON_OTHERS		4

typedef struct _nil_task  nil_task_t;

struct _nil_task_manager {
	nlist_t *head;
	nlist_t *saved;
	struct nlist_op *listOp;
	int (*addTask)(struct _nil_task_manager *mg, nil_task_t *task);
	nil_task_t* (*nextTask)(struct _nil_task_manager *mg);
	int (*delTask)(struct _nil_task_manager *mg, nil_task_t *task);
};

typedef struct _nil_task_manager nil_task_mgm_t;

extern int initTaskManager (nil_task_mgm_t **tmgm);
extern int addTask2TaskMgm (nil_task_mgm_t *tmgm, nil_task_t *task);
extern nil_task_t *getNext (nil_task_mgm_t *tmgm);
extern void destroy_TaskManager (nil_task_mgm_t **tmgm);

extern int creatTaskOnCore(void **tid,void*(*run)(void *),void* arg,int core);
extern int getNumOfCores(int *core);
extern int createTask(nil_task_t *taskarg);
extern int waitAllTasks(nil_task_mgm_t *tmgm);

extern int initTask(nil_task_t **task);
extern int destroyTask(nil_task_t *task);

extern int getNumOfCores(int *core);

#ifdef __cplusplus
}
#endif

#endif

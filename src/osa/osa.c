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
#if defined (__linux__)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined (__linux__) || defined (__APPLE__)
#include <fts.h>
#elif defined (__MINGW32__)
#include <ftw.h>
#endif
#include <signal.h>
#include <errno.h>
#include "nein/osa.h"
#include "nein/debug.h"

static int sysCore = 0;

static void *_task_work(void *taskarg);
static int creatTask(nil_task_t *taskarg);

int assign2core(int run_on, int numCore) ;

int setSigHandler(sighandle_set *sigset)
{
	int	rc = 0;
#if defined(__linux__) || defined (__APPLE__)
	struct sigaction sa;
	sa.sa_handler = (void (*)(int))(sigset->hndl);
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	if (0!=(rc=sigaction (sigset->signo, &sa, NULL)))
	{
		rc = -errno;
	}
#elif defined (__MINGW32__)
	signal (sigset->signo, sigset->hndl);
#endif
	return rc;
}

#if defined (__linux__) || defined (__APPLE__)
static int fts_comp(const FTSENT **l, const FTSENT **r)
{
	FTSENT *lp, *rp;
	int rv = -1;

	if ( !l || !r || !*l || !*r ) return rv;

	lp = (FTSENT*)*l;
	rp = (FTSENT*)*r;

	rv = strcmp (lp->fts_path, rp->fts_path);

	if (rv != 0) return rv;

	return strcmp(lp->fts_name, rp->fts_name);
}
#endif
#if defined (__MINGW32__)
typedef void(*ftn_call)(const char *path);

static ftn_call walking_dir_doing = NULL;
int _ftw_fn (const char* path, const struct stat *st, int itype, struct FTW *sftw)
{
	NIL_DEBUG ("PATH: %s\n", path);
	if (itype == FTW_F) {
		if (walking_dir_doing){
			walking_dir_doing (path);
		}
	}
	return 0;
}
#endif

static int _addTask (nil_task_mgm_t *mg, nil_task_t *task)
{
	int rc = 0;
	if (mg && task && mg->listOp){
		return mg->listOp->append(&mg->head, NULL, (nlist_t*)task);
	}
	return -EINVAL;
}

static nil_task_t* _nextTask (nil_task_mgm_t *mg)
{
	if (mg && mg->listOp){
		return (nil_task_t*)(mg->listOp->next(&mg->head, NULL, &mg->saved));
	}
	return NULL;
}

static int _delTask (nil_task_mgm_t *mg, nil_task_t *task)
{
	int rc = 0;
	if (mg && task && mg->listOp){
		return mg->listOp->remove(&mg->head, NULL, (nlist_t*)task);
	}
	return -EINVAL;
}

int walkThDir(char* dpath, void(*doing)(const char *fpath))
{
	int rc = 0;
#if defined (__linux__) || defined (__APPLE__)
	char *fts_arg[2] = {NULL, NULL};
	FTS *pfts = NULL;
	FTSENT	*pfte = NULL;
	FTSENT  *pchild = NULL;
	int fts_option = FTS_NOCHDIR ; //FTS_LOGICAL|FTS_NOCHDIR; 
	NIL_DEBUG("try open %s\n", dpath);
	fts_arg[0] = dpath;

	if (!(pfts = fts_open (fts_arg, fts_option, fts_comp)) ){
		NIL_ERROR("fts_open error. \n");
		return 2;
	}
	while (NULL!=(pfte=fts_read(pfts)) ) {
		if (pfte->fts_info == FTS_F && doing) {
			NIL_DEBUG("doing something %s\n", pfte->fts_path);
			doing ((const char*)pfte->fts_path);
		}
	}
	fts_close (pfts);
#elif defined (__MINGW32__)
	walking_dir_doing = doing;
	NIL_DEBUG ("%s:%d\n", __FILE__, __LINE__);
	if (0!= (rc = nftw(dpath, _ftw_fn, 2048, 0))) {
		NIL_ERROR("nftw returns -1\n");
		rc = - errno;
	}
#endif
	return rc; 
}


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
pthread_attr_t *setCore (pthread_attr_t *iattr, int onCore)
{
	pthread_attr_t *pattr = NULL;
#if defined(_LINUX_)
	cpu_set_t	cpuset;

	CPU_ZERO(&cpuset);
	if (onCore >= sysCore){
		NIL_DEBUG("input value is over sysCore. addjust to %d\n", onCore%sysCore);
		onCore %= sysCore;
	}
	CPU_SET(onCore, &cpuset);
	if (NULL == iattr) {
		if(!(pattr = (pthread_attr_t *)malloc(sizeof(pthread_attr_t)))) 
		{
			NIL_ERROR("Couldn't allocate for pthread_attr_t\n");
			return NULL;
		}
		pthread_attr_init(pattr);
	} else {
		pattr = iattr;
	}
	if (0 != pthread_att_setaffinity_np(&pattr, sizeof(cpu_set_t), &cpuset)){
		if (iattr & pattr){
			NIL_DEBUG("Could not setaffinity... \n");
			pthread_attr_destroy(pattr);
			pattr = NULL;
		}
	}
#elif defined(_WIN32_) && defined(PTHREAD_W32)
#else
#endif
	return pattr;
}

int getNumOfCores(int *core)
{
	int nCore = 1;
#if defined(__linux__) || defined(__APPLE__)
	nCore = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined (__MINGW32__) 
	nCore = pthread_num_processors_np();
	NIL_DEBUG("%d core%s\n", nCore, nCore>1?"s":"");
#endif
	if(nCore < 1) nCore = 1;
	*core = sysCore = nCore;
	return nCore;
}

int createTask(nil_task_t *taskarg)
{
	int ret = 0;
	pthread_t tid = (pthread_t)0;
	pthread_attr_t	*attr = NULL;
	size_t		stackSize = (1*1024*1024);
	int		attr_allocated_in_this_function = 0;
#if defined(__USE_XOPEN2K) || defined (__MINGW32__)
	pthread_barrier_t *barrier = NULL;
#endif
	if (NULL == taskarg){
		return -EINVAL;
	}

	if (taskarg->attr)
		attr = taskarg->attr;
	if (NULL == attr)
	{
		attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
		ret = pthread_attr_init (attr);
		attr_allocated_in_this_function = 1;
	}
	/** set stack size */
	ret = pthread_attr_setstacksize (attr, stackSize);
	if (-1 != taskarg->sched_policy) {
		if (-1 != taskarg->sched_priority){
				struct sched_param schedparam = {0,};
				schedparam.sched_priority  = taskarg->sched_priority;
				ret |= pthread_attr_setschedpolicy(attr, taskarg->sched_policy);
				ret |= pthread_attr_setschedparam(attr, &schedparam);
		}
	}
#if defined(__linux__)
	if ( -1 != taskarg->onCore ) {
		(void)assign2coreInMain( RUN_ON_LINUX, taskarg->onCore, &attr);
	}
#endif

	ret |= pthread_create(&tid, attr, _task_work, (void*)taskarg);
	if (!ret)
		taskarg->tid = tid;
	if (attr_allocated_in_this_function) {
		pthread_attr_destroy (attr);
		free (attr);
	}
	return ret;
}

void *_task_work(void *arg)
{
#if defined (__USE_XOPEN2K) || defined (__MINGW32__)
	pthread_barrier_t *barrier = NULL;
#endif
	nil_task_t *taskarg = NULL;
	if (!arg) {
		NIL_DEBUG("%s[%d] null argument.\n",__FUNCTION__,__LINE__);
		return (void*)0;
	}
	taskarg = (nil_task_t*)arg;
#if defined (__MINGW32__)
	if (taskarg->onCore > -1)
		(void)assign2core(RUN_ON_WIN_MINGW, taskarg->onCore);
#endif
#if defined (__USE_XOPEN2K) || defined (__MINGW32__)
	if (taskarg->barrier)
		barrier = taskarg->barrier;
	if (barrier) 
		pthread_barrier_wait(barrier);
#endif
	return (void*)taskarg->work_run(taskarg->private);
}


int assign2core(int run_on, int numCore) 
{
	int rc = 0;
#if defined (__linux__)
	if (RUN_ON_LINUX != run_on) return -1;

#elif defined(__MINGW32__)
	if (RUN_ON_WIN_MINGW != run_on) return -1;
	rc = pthread_set_num_processors_np(numCore);
#endif
	return rc;
}

int assign2coreInMain(int os_run, int numCore, pthread_attr_t **ppattr)
{
	int rc = 0;
#if defined (__linux__) && defined (_GNU_SOURCE)
	pthread_attr_t *attr = NULL;
	cpu_set_t cpuset;
	if (RUN_ON_LINUX != os_run) {
		return -1;
	}
	/* if (numCore < 0) return -1; */
	attr = *ppattr;
	if (NULL==attr) {
		attr = (pthread_attr_t*)malloc(sizeof(pthread_attr_t));
		if (NULL == attr) {
			return -1;
		}
		if(0!=pthread_attr_init(attr)) {
			free(attr);
			return -1;
		}
		*ppattr = attr;
	}
	CPU_ZERO(&cpuset);
	CPU_SET(numCore,&cpuset);
	rc = pthread_attr_setaffinity_np(attr,sizeof(cpu_set_t),&cpuset);
#elif defined(__MINGW32__)
	if (RUN_ON_WIN_MINGW != os_run) {
		return -1;
	}
	rc = pthread_set_num_processors_np(numCore);
#else
	#warning "Not supported: assigning core"
#endif
	return rc;
}

int waitAllTasks(nil_task_mgm_t *mgm)
{
	int rc = 0;
	nil_task_t *task = NULL;
	if (!mgm) return -EINVAL;
	task = mgm->nextTask(mgm);
	while (task) {
		rc = pthread_join(task->tid, NULL);
		task = mgm->nextTask(mgm);
	}
	return 0;
}

int initTaskManager (nil_task_mgm_t **tmgm)
{
	if (tmgm) {
		nil_task_mgm_t *p = (nil_task_mgm_t*)malloc(sizeof(nil_task_mgm_t));
		if (!p) return -ENOMEM;
		memset (p, 0x00, sizeof(nil_task_mgm_t));
		p->listOp = &cl_op;
		p->addTask = _addTask;
		p->nextTask = _nextTask;
		p->delTask = _delTask;
		*tmgm = p;
		return 0;
	}
	return -EINVAL;
}

int addTask2TaskMgm (nil_task_mgm_t *tmgm, nil_task_t *task)
{
	if (tmgm && task && tmgm->addTask) {
		return tmgm->addTask (tmgm, task);
	}
	return -EINVAL;
}

nil_task_t *getNext (nil_task_mgm_t *tmgm)
{
	if (tmgm && tmgm->nextTask) {
		return tmgm->nextTask(tmgm);
	}
	return NULL;
}

void destroy_TaskManager (nil_task_mgm_t **tmgm)
{
	if (tmgm && *tmgm){
		nil_task_t *task = NULL;
		(*tmgm)->saved = NULL;
		task = (*tmgm)->nextTask (*tmgm);
		while (task) {
			(*tmgm)->delTask (*tmgm, task);
			(void)destroyTask(task);
			(*tmgm)->saved = NULL;
			task = (*tmgm)->nextTask (*tmgm);
		}
		free (*tmgm);
		*tmgm = NULL;
	}
}

int initTask (nil_task_t **task)
{
	if (task) {
		nil_task_t *t = NULL;
		t = (nil_task_t*)malloc(sizeof(nil_task_t));
		if (!t) return -ENOMEM;
		memset(t, 0x00, sizeof(nil_task_t));
		t->sched_priority = t->sched_policy = -1;
		t->onCore = -1;
		*task = t;
		return 0;
	}
	return -EINVAL;
}

int destroyTask (nil_task_t *task)
{
	if (task){
		free (task);
		return 0;
	}
	return -EINVAL;
}


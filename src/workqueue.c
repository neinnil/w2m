
/***
    WorkQueue.c
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
***/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include "nein/debug.h"

#include "workqueue.h"


#define LOCK(x)		pthread_mutex_lock((x)->mu)
#define UNLOCK(x)	pthread_mutex_unlock((x)->mu)


static int _add2list(workqueue_t* wq, nlist_t **head, nlist_t *item)
{
	if (!wq || !item) return -1;
	if (wq->op && wq->op->append) {
		return wq->op->append(head, NULL, item); 
	}
	return -1;
}

static int _rmItem(workqueue_t* wq, nlist_t **head, nlist_t *item)
{
	if (wq->op && wq->op->remove) {
		return wq->op->remove (head, NULL, item);
	}
	return -1;
}

static int _init(workqueue_t* wq, nlist_t *item)
{
	if (wq->op && wq->op->init) {
		return wq->op->init (item);
	}
	return -1;
}

static int _exist(workqueue_t* wq, nlist_t **head, nlist_t *item)
{
	if (wq->op && wq->op->exist) {
		return wq->op->exist (head, item);
	}
	return -1;
}

static void _swap (workqueue_t* wq, nlist_t **head, nlist_t *x, nlist_t *y)
{
	if (wq->op && wq->op->swap) {
		wq->op->swap (head, NULL, x, y);
	}
}

static nlist_t *_next (workqueue_t *wq, nlist_t **head, nlist_t **cur)
{
	if (wq->op && wq->op->next) {
		return wq->op->next (head, NULL, cur);
	}
	return NULL;
}

static nlist_t *_search (workqueue_t *wq, nlist_t **head, void *key, int (*comp)(void*, void*)) 
{
	if (wq->op && wq->op->search) {
		return wq->op->search (head, NULL, key, comp);
	}
	return NULL;
}

workitem_t * allocWorkItem (void *priv, int nSize)
{
	workitem_t *work = NULL;

	if (nSize < 0) {
		nSize = 0;
	}

	work = (workitem_t*)malloc (sizeof(workitem_t));
	if (!work) {
		return NULL;
	}
	memset (work, 0x00, sizeof(workitem_t));
	work->mu = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (!work->mu) {
		goto Error;
	}
	if (0!=pthread_mutex_init (work->mu, NULL)) {
		free (work->mu);
		work->mu = NULL;
		goto Error;
	}
	if (nSize && priv) {
		work->priv = (void*)malloc(nSize);
		if (!work->priv) {
			goto Error;
		}
		memcpy (work->priv, priv, nSize);
	}
	return work;
Error:
	if (work->priv) {
		free(work->priv);
	}
	if (work->mu) {
		pthread_mutex_destroy (work->mu);
		free (work->mu);
	}
	free(work);
	return NULL;
}

int trylock_workItem(workitem_t *wit)
{
	if (!wit) return -EINVAL;
	if (!wit->mu) return -EINVAL;
	return pthread_mutex_trylock(wit->mu);
}

int lock_workItem(workitem_t *wit)
{
	if (!wit) return -EINVAL;
	if (!wit->mu) return -EINVAL;
	return pthread_mutex_lock(wit->mu);
}

int unlock_workItem(workitem_t *wit)
{
	if (!wit) return -EINVAL;
	if (!wit->mu) return -EINVAL;
	return pthread_mutex_unlock(wit->mu);
}

int destroyWorkItem (workqueue_t *wq, nlist_t** head, workitem_t *wit)
{
	int rc = 0;
	if (!wq || !wit || !head || !*head) return -EINVAL;

	rc = pthread_mutex_lock (wit->mu);

	_rmItem (wq, head, (nlist_t*)wit);

	if (wq->free_private) {
		wq->free_private (wit->priv);
	} else {
		free (wit->priv);
	}

	pthread_mutex_unlock (wit->mu);
	pthread_mutex_destroy(wit->mu);
	free (wit);
	return 0;
}

int init_wq (workqueue_t **wq)
{
	int rc = 0;
	if (!wq) {
		return -EINVAL;
	}
	if (!*wq){
		*wq = (workqueue_t*)malloc(sizeof(workqueue_t));
		if (!*wq) return -ENOMEM;
	}
	(*wq)->mu = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(!(*wq)->mu)
		return -ENOMEM;
	if ( 0!=(rc=pthread_mutex_init((*wq)->mu, NULL)) ) {
		free ((*wq)->mu);
		return -rc;
	}
	(*wq)->nTotal = (*wq)->nFree = (*wq)->nDoing = (*wq)->nDone = 0;
	(*wq)->free = (*wq)->doing = (*wq)->done = NULL;
	(*wq)->curFree = (*wq)->curDoing = (*wq)->curDone = NULL;
	(*wq)->op = &cl_op;
	(*wq)->free_private = NULL;
	(*wq)->complete = NULL;
	return 0;
}

static void _destroy_list(workqueue_t *wq, nlist_t **head)
{
	int rc = 0;
	workitem_t	*wit = NULL;
	nlist_t		*saved = NULL;

	if (!wq || !head || !*head) return ;

	wit = (workitem_t*)_next (wq, head, &saved);
	while (wit) {
		destroyWorkItem (wq, head, wit);
		saved = NULL;
		wit = (workitem_t*)_next (wq, head, &saved);
	}
}

int destroy_wq (workqueue_t **wq)
{
	int rc = 0;
	workitem_t	*wit = NULL;
	int			i;
	if (!wq || !*wq) return -EINVAL;
	pthread_mutex_lock((*wq)->mu);

	_destroy_list ( (*wq), &(*wq)->free);
	_destroy_list ( (*wq), &(*wq)->doing);
	_destroy_list ( (*wq), &(*wq)->free);

	(*wq)->free = (*wq)->doing = (*wq)->done = NULL;
	(*wq)->curFree = (*wq)->curDoing = (*wq)->curDone = NULL;

	pthread_mutex_unlock((*wq)->mu);
	rc = pthread_mutex_destroy((*wq)->mu);

	free ((*wq));
	*wq = NULL;
	return 0;
}

int addNewItem(workqueue_t *wq, workitem_t *it)
{
	int rc = 0;
	pthread_mutex_lock(wq->mu);
	rc = _add2list (wq, &wq->free, (nlist_t*)it);
	if (rc==0) {
		wq->nTotal++;
		wq->nFree++;
	}
	pthread_mutex_unlock(wq->mu);
	return rc;
}

int addItem2Doing(workqueue_t *wq, workitem_t *it)
{
	int rc = 0;
	pthread_mutex_lock(wq->mu);
	rc = _rmItem (wq, &wq->free, (nlist_t*)it);
	if (rc < 0) {
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	rc = _add2list (wq, &wq->doing, (nlist_t*)it);
	if (rc < 0) {
		rc = _add2list (wq, &wq->free, (nlist_t*)it);
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	wq->nFree--;
	wq->nDoing++;
	wq->curFree = NULL;
	wq->curDoing = NULL;
	wq->curDone = NULL;

	pthread_mutex_unlock(wq->mu);
	return rc;
}

int addItem2Done(workqueue_t *wq, workitem_t *wit)
{
	int rc = 0;
	pthread_mutex_lock(wq->mu);
	rc = _rmItem (wq, &wq->doing, (nlist_t*)wit);
	if (rc < 0) {
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	rc = _add2list (wq, &wq->done, (nlist_t*)wit);
	if (rc < 0) {
		rc = _add2list (wq, &wq->doing, (nlist_t*)wit);
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	wq->nDoing--;
	wq->nDone++;

	wq->curFree = NULL;
	wq->curDoing = NULL;
	wq->curDone = NULL;

	rc = pthread_mutex_trylock (wit->mu);
	if (rc ==EBUSY) {
		NIL_DEBUG("%s:%d Already locked.\n", __FILE__, __LINE__);
	}
	pthread_mutex_unlock (wit->mu);
	pthread_mutex_unlock(wq->mu);
	if (wq->nTotal == wq->nDone && wq->complete) {
		wq->complete();
	}
	return rc;
}

static workitem_t *_getNextWorkItem (workqueue_t *wq, nlist_t** head, nlist_t** saved)
{
	int			rc = 0;
	workitem_t *wit = NULL;
	if (!wq || !head || !*head) return NULL;
	rc = pthread_mutex_lock(wq->mu);
	wit = (workitem_t*)_next (wq, head, saved);
	rc = pthread_mutex_unlock(wq->mu);
	return wit;
}

workitem_t *getNextFreeWork (workqueue_t *wq)
{
	return _getNextWorkItem (wq, &wq->free, &wq->curFree);

}

workitem_t *getNextDoingWork (workqueue_t *wq)
{
	return _getNextWorkItem (wq, &wq->doing, &wq->curDoing);
}

workitem_t *getNextDoneWork (workqueue_t *wq)
{
	return _getNextWorkItem (wq, &wq->done, &wq->curDone);
}

workitem_t *popWorkFromFree (workqueue_t *wq)
{
	int			rc = 0;
	workitem_t *wit = NULL;
	if (!wq) return NULL;
	rc = pthread_mutex_lock(wq->mu);
	do {
		if (1 != _exist (wq, &wq->free, wq->curFree)) {
			wq->curFree = NULL;
		}
		wit = (workitem_t*)_next (wq, &wq->free, &wq->curFree);
		if (wit == NULL) break;
		rc = pthread_mutex_trylock (wit->mu);
		if (rc == EINVAL){
			rc = 0;
			wit = NULL;
		}
	} while (rc != 0);
	pthread_mutex_unlock(wq->mu);
	return wit;
}

int getNumbState (workqueue_t *wq, int *nTotal, int *nFree, int *nDoing, int *nDone)
{
	if (!wq) return -1;
	pthread_mutex_lock(wq->mu);
	*nTotal	=	wq->nTotal;
	*nFree	=	wq->nFree;
	*nDoing	=	wq->nDoing;
	*nDone	=	wq->nDone;
	pthread_mutex_unlock(wq->mu);
	return 0;
}

int setFreeFn(workqueue_t *wq, void (*freefn)(void*))
{
	if (!wq || !freefn) return -EINVAL;
	wq->free_private = freefn;
	return 0;
}

void setCompleteFn (workqueue_t *wq, void (*complete)(void))
{
	if (wq && complete) {
		wq->complete = complete;
	}
}

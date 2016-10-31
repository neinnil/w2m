
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

#include "nein/list.h"

struct workitem {
	nlist_t	list;
	pthread_mutex_t *mu;
	pthread_t		*tid;
	int				state;
	void			*priv;
};

struct workqueue {
	nlist_t	*whole;
	nlist_t	*free;
	nlist_t	*doing;
	nlist_t	*done;
	nlist_t *curFree;
	pthread_mutex_t	*mu;

	int				nTotal;
	int				nFree;
	int				nDoing;
	int				nDone;

	nlist_t_op	*op;
};

typedef struct workqueue workqueue_t;
typedef struct workitem	 workitem_t;

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

static int  _exist(workqueue_t* wq, nlist_t **head, nlist_t *item)
{
	if (wq->op && wp->op->exist) {
		return wp->op->exist (head, item);
	}
	return -1;
}

static void _swap (workqueue_t* wq, nlist_t **head, nlist_t *x, nlist_t *y)
{
	if (wq->op && wq->op->swap) {
		return wp->op->swap (head, NULL, x, y);
	}
	return ;
}

static nlist_t *_next (workqueue_t *wq, nlist_t **head, nlist_t **cur)
{
	if (wq->op && wq->op->next) {
		return wq->op->next (head, NULL, cur)
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

int init_wq (workqueue_t **wq)
{
	int rc = 0;
	if (wq || *wq) {
		return -1;
	}
	wq->mu = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(!wq->mu)
		return -1;
	if ( 0!=(rc=pthread_mutex_init(wq->mu, NULL)) ) {
		free (wq->mu);
		return -rc;
	}
	wq->nTotal = wq->nFree = wq->nDoing = wq->nDone = 0;
	wq->whole = wq->free = wq->doing = wq->done = wq->curFree = NULL;
	wq->op = &cl_op;
	return 0;
}

int addNewItem(workqueue_t *wq, workitem_t *it)
{
	int rc = 0;
	pthread_mutex_lock(wq->mu);
	rc = _add2list (wq, &wq->whole, (nlist_t*)it);
	if (rc < 0) {
		pthread_mutex_unlock(wq->mu);
		return rc;
	}
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
	rc = _add2list (wq, &wq->doing, (nlist_t*)it);
	if (rc < 0) {
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	rc = _rmItem (wq, &wq->free, it);
	if (rc < 0) {
		rc = _rmItem (wq, &wq->doing, it);
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	wq->nFree--;
	wq->nDoing++;

	pthread_mutex_unlock(wq->mu);
	return rc;
}

int addItem2Done(workqueue_t *wq, workitem_t *it)
{
	int rc = 0;
	pthread_mutex_lock(wq->mu);
	rc = _add2list (wq, &wq->done, (nlist_t*)it);
	if (rc < 0) {
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	rc = _rmItem (wq, &wq->doing, it);
	if (rc < 0) {
		rc = _rmItem (wq, &wq->done, it);
		pthread_mutex_unlock(wq->mu);
		return -1;
	}
	wq->nDoing--;
	wq->nDone++;

	pthread_mutex_unlock(wq->mu);
	return rc;
}

workitem_t *getNextFreeWork (workqueue_t *wq)
{
	workitem_t *wit;
	if (!wq) return -1;
	pthread_mutex_lock(wq->mu);
	wit = _next (wq, &wq->free, &wq->curFree);
	pthread_mutex_unlock(wq->mu);
	return wit;
}

int getNumbState (workqueue_t *wq, int *nTotal, int *nFree, int *nDoing, int *nDone)
{
	if (!wq) return -1;
	pthread_mutex_lock(wq->mu);
	nTotal	=	wq->nTotal;
	nFree	=	wq->nFree;
	nDoing	=	wq->nDoing;
	nDone	=	wq->nDone;
	pthread_mutex_unlock(wq->mu);
	return 0;
}

int initWorkItem (void *priv, int nSize)
{
	workitem_t *work = NULL;

	if (nSize < 0) {
		nSize = 0;
	}

	work = (workitem_t*)malloc (sizeof(workitem_t));
	if (!work) {
		return -1;
	}
	memset (work, 0x00, sizeof(workitem_t));
	work->mu = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if ( !work->mu ) {
		goto Error;
	}
	pthread_mutex_init (work->mu);
	if (nSize && priv) {
		work->priv = (void*)malloc(nSize);
		if (!work->priv) {
			goto Error;
		}
		memcpy (work->priv, priv, nSize);
	}
Error:
	if (work->priv) free(work->priv);
	if (work->mu) {
		pthread_mutex_destroy (work->mu);
		free (work->mu);
	}
	free(work);
	return -1;
}


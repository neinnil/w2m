
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

#ifndef _WORKQUEUE_H
#define _WORKQUEUE_H

#include <pthread.h>

#include "nein/list.h"

#ifdef __cplusplus
extern "C" {
#endif

struct workitem {
	nlist_t	list;
	pthread_mutex_t	*mu;
	pthread_t		*tid;
	int				state;
	void			*priv;
};

struct workqueue {
	nlist_t	*free;
	nlist_t	*doing;
	nlist_t	*done;
	nlist_t *curFree;
	pthread_mutex_t	*mu;

	int				nTotal;
	int				nFree;
	int				nDoing;
	int				nDone;

	struct nlist_op	*op;
	void (*free_private)(void *);
};

typedef struct workqueue workqueue_t;
typedef struct workitem	 workitem_t;

extern workitem_t*	allocWorkItem (void *priv, int nSize);
extern int destroyWorkItem (workqueue_t *wq, nlist_t** head, workitem_t *wit);

extern int init_wq (workqueue_t **wq);
extern int destroy_wq (workqueue_t **wq);

extern int addNewItem(workqueue_t *wq, workitem_t *it);
extern int addItem2Doing(workqueue_t *wq, workitem_t *it);
extern int addItem2Done(workqueue_t *wq, workitem_t *it);
extern workitem_t*	getNextFreeWork (workqueue_t *wq);
extern int getNumbState (workqueue_t *wq, int *nTotal, int *nFree, int *nDoing, int *nDone);

extern int setFreeFn(workqueue_t *wq, void (*freefn)(void*));

#ifdef __cplusplus
}
#endif

#endif

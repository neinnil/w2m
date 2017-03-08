/**
## Not Use
    queue.c
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
**/

#include <pthread.h>
#include "nein/list.h"
#include "nein/queue.h"

struct nqueue_it {
	struct nlist list;
	uint8_t	priv_data[1];
};

struct nqueue {
	pthread_mutex_t	*mu;
	size_t			count;
	struct nlist	*head, *tail, *curr;
};

typedef struct nqueue * nqhnd_t;

static struct nqueue_it * alloc_nq_item(void * userdata, size_t userSize);
static int _enqueue_item(struct nqueue *nq, struct nqueue_it * item);

static int nq_lock(nqhdn_t nq)
{
	int ret = 0;
	if (NULL == nq) return -EINVAL;
	if (NULL == nq->mu) {
		nq->mu = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		if (!nq->mu) return -ENOMEM;
		if(0!=(ret = pthread_mutex_init(nq->mu, NULL)))
			return -ret;
	}
	ret = pthread_mutex_lock(nq->mu);
	return ret;
}

static int nq_unlock(struct nqueue *nq)
{
	if (NULL == nq) return -EINVAL;
	return pthread_mutex_unlock(nq->mu);
}

int enqueue (void *udata, size_t usize)
{
	int ret = 0;
	struct nqueue_it *item = NULL;
	ret = alloc_list (&item, sizeof(struct nqueue_it) + uszie)
	ret = nq_lock ();
	memcpy (&item->priv_data[0], udata, usize);
	ret = nq_unlock ();
	return 0;
}

nqueue_t * dequeue(struct nqueue *nq)
{
	nqueue_t *q = NULL;
	int ret = 0;
	ret = nq_lock ();
	ret = nq_unlock ();
	return q;
}

nqueue_t * get (struct nqueue *nq, int idx)
{
}

nqueue_t * next (struct nqueue *nq)
{
}

int init_queue (struct nqueue **que)
{
	struct nqueue *nq = NULL;
	if (NULL == que) return -EINVAL;

	nq = (struct nqueue*)malloc(sizeof(struct nqueue));
	memset (nq, 0x00, sizeof(struct nqueue));
	*que = nq;
	return 0;
}

size_t count_queue(struct nqueue *nq)
{
	size_t nSize = (size_t)-1;
	if (NULL!=nq)
		nSize = nq->count;
	return nSize;
}

int purge_queue (struct nqueue **nq)
{
}

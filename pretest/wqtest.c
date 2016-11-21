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

#include "workqueue.h"

#define LIST_OF_FREE	0
#define LIST_OF_DOING	1
#define LIST_OF_DONE	2

typedef workitem_t* (*nextWork)(workqueue_t *) ;

nextWork nextItem[3] = {
	getNextFreeWork,
	getNextDoingWork,
	getNextDoneWork
};

static int nTotal, nFree, nDoing, nDone ;

void print_item (workitem_t *wit)
{
	printf ("%6d | %20p | %20p | %20p \n",
			*(int*)(wit->priv), wit, wit->list.prev, wit->list.next);
}

void print_list (workqueue_t *wq, int listType, int limit)
{
	workitem_t *wit = NULL;
	
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	wit = nextItem[listType](wq);
	while (wit && limit) {
		print_item(wit);
		wit = nextItem[listType](wq);
		limit--;
	}
}

void print_allList (workqueue_t *wq)
{
	printf ("Free working list\n");
	print_list (wq, LIST_OF_FREE, nFree);
	printf ("Doing working list\n");
	print_list (wq, LIST_OF_DOING, nDoing);
	printf ("Done working list\n");
	print_list (wq, LIST_OF_DONE, nDone);

	wq->curFree = wq->curDoing = wq->curDone = NULL;
}

void print_statics(workqueue_t *wq)
{
	if(0==getNumbState (wq, &nTotal, &nFree, &nDoing, &nDone)){
		printf ("Total: %d, Free: %d, Doing: %d, Done: %d\n", 
				nTotal, nFree, nDoing, nDone);
	}
}

int main (int ac, char** av)
{
	int rc = 0;
	workqueue_t	*wq = NULL;
	workitem_t	*wit = NULL;

	rc = init_wq(&wq);
	printf ("init_wq returns %d:%s\n", rc, rc==0?"":strerror(-rc));
	assert(0==rc);

	for (ac = 0; ac < 5 ; ac++){
		rc = 0;
		wit = allocWorkItem ((void*)&ac, sizeof(int));
		if (wit){
			rc = addNewItem (wq, wit);
		}
		if (!wit || rc) {
			printf ("Fail: %s\n", !wit?"allocation":"addition");
			break;
		}
		print_statics(wq);
	}
print_allList(wq);
	wit = getNextFreeWork(wq);
	addItem2Doing (wq, wit);
	printf ("\n\n");
	print_statics(wq);
print_allList(wq);
	printf ("\n\n");
	addItem2Done (wq, wit);
	print_statics(wq);
print_allList(wq);
	for (ac=0; ac < nTotal; ac++) {	
		wit = popWorkFromFree (wq);
		printf("popWorkFromFree returns : %p\n", wit);
	}
extern int unlock_workItem(workitem_t *); 
	wq->curFree = NULL;
	for (ac=0; ac<nFree; ac++) {
		wit = getNextFreeWork(wq);
		unlock_workItem (wit);
	}

printf ("Destroy wq\n");
	rc = destroy_wq(&wq);

	return 0;
}

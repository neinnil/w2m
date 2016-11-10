
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

void print_statics(workqueue_t *wq)
{
	int nTotal, nFree, nDoing, nDone ;
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

	wit = getNextFreeWork(wq);
	addItem2Doing (wq, wit);
	print_statics(wq);
	addItem2Done (wq, wit);
	print_statics(wq);

	rc = destroy_wq(&wq);

	return 0;
}

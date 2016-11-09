/*
    linked list
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
#include <stdlib.h>
#include <assert.h>
#include "nein/list.h"


struct mlist {
	struct nlist *prev, *next;
	int v;
};

void print_item (struct mlist *item)
{
	printf ("%6d | %20p | %20p | %20p\n",
			item->v, item, item->prev, item->next );
}
	
void print_list(struct mlist *head) 
{
	int count = 0;
	struct mlist *tr = head;
	if (!head) {
		printf ("There is no item.\n");
		return ;
	}
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	while (tr) {
		print_item (tr);
		tr = (struct mlist*)tr->next;
		if(tr == head) break;
	}
}

void print_all_items (struct nlist *head)
{
	struct mlist *next_item = NULL;
	struct nlist *save = NULL;
	if (!head) {
		printf ("There is no item.\n");
		return ;
	}
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	next_item = (struct mlist*)cl_op.next(&head, NULL, &save);
	while (next_item) {
		print_item (next_item);
		next_item = (struct mlist*)cl_op.next(&head, NULL,  &save);
	}
}

int int_comp (void *li, void *value)
{
	struct mlist *lv = (struct mlist *)li;
	int			val = *(int *)value;	
	if (!lv) return -1;
	return (lv->v) - val;
}

void search_test (struct mlist **head, int search_key)
{
	struct nlist *item = NULL;

	item = cl_op.search ((struct nlist**)head, NULL, (void*)&search_key, int_comp);
	if (item) {
		printf ("Found. (%d)\n", search_key);
		print_item((struct mlist*)item);
	} else {
		printf ("Cannot find. (%d)\n", search_key);
	}
}

struct mlist * allocItem(int ival)
{
	struct mlist *p = NULL;
	p = (struct mlist*)malloc(sizeof(struct mlist));
	if (p) {
		p->v = ival;
	}
	return p;
}

void destroyItem (struct mlist **head, struct mlist *it)
{
	if (!head || !*head){
		assert (0);
	}
	if (it) {
		printf("Remove: ");
		print_item (it);
		cl_op.remove ((nlist_t**)head, NULL, (nlist_t*)it);
		free (it);
	}
}

void destroyAllItems(struct mlist **head)
{
	struct nlist* item = NULL;
	nlist_t*	save = NULL;
	item = cl_op.next ((nlist_t**)head, NULL, &save);
	while (item) {
		destroyItem (head, (struct mlist*)item);
		printf("Remains: \n");
		print_list(*head);
		save = NULL;
		item = cl_op.next ((nlist_t**)head, NULL, &save);
	}
}


int main (int ac, char** av)
{
	struct mlist *head = NULL;
	struct mlist *it = NULL;
	/* add items 0 ~ 3 */
	
	for ( ac = 0; ac < 4 ; ac++ ) {
		it = allocItem (ac);
		cl_op.append((nlist_t**)&head, NULL, (nlist_t*)it);
	}
	print_list (head);

	printf ("To Try Destroy All Items.\n");
	destroyAllItems(&head);
	printf ("Done... \n");
	print_list (head);
	return 0;
}


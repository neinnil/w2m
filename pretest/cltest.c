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


int main (int ac, char** av)
{
	struct mlist *head = NULL;
	struct mlist a ={ NULL, NULL, 1 };
	struct mlist b ={ NULL, NULL, 2 };
	struct mlist c ={ NULL, NULL, 3 };
	struct mlist d ={ NULL, NULL, 4 };
	struct mlist e ={ NULL, NULL, 5 };
	struct mlist f ={ NULL, NULL, 6 };
	struct mlist g ={ NULL, NULL, 7 };
	struct mlist h ={ NULL, NULL, 8 };

	cl_op.append((nlist_t**)&head, NULL, (nlist_t*)&a);
	printf ("Address of head: %p\n", head);
	print_list (head);

	printf ("Add 2, expected: 1 2\n");
	cl_op.append((nlist_t**)&head, NULL, (nlist_t*)&b);
	print_list (head);

	printf("Swap 1, 2, expected: 2 1\n");
	cl_op.swap ((nlist_t**)&head, NULL,(nlist_t*)&a, (nlist_t*)&b);
	print_list (head);

	printf ("Swap 1, 2, expected: 1 2\n");
	cl_op.swap ((nlist_t**)&head, NULL, (nlist_t*)&a, (nlist_t*)&b);
	print_list (head);

	printf ("Add 3, expected: 1 2 3\n");
	cl_op.append((nlist_t**)&head, NULL, (nlist_t*)&c);
	print_list(head);

	printf("Swap 1, 2, expected: 2 1 3\n");
	cl_op.swap ((nlist_t**)&head, NULL, (nlist_t*)&a, (nlist_t*)&b);
	print_list (head);

	printf("Swap 1, 2, expected: 1 2 3\n");
	cl_op.swap ((nlist_t**)&head, NULL, (nlist_t*)&a, (nlist_t*)&b);
	print_list (head);

	printf("Del 2, expected: 1 3\n");
	cl_op.remove ((nlist_t**)&head, NULL, (nlist_t*)&b);
	print_list(head);

	printf("Add 2, expected: 1 3 2\n");
	cl_op.append ((nlist_t**)&head, NULL, (nlist_t*)&b);
	print_list(head);

	printf ("Del 1, expected: 3 2\n");
	cl_op.remove ((nlist_t**)&head, NULL, (nlist_t*)&a);
	print_list(head);

	printf("Add 1, Swap 3, 1,  expected: 1 3 2\n");
	cl_op.append((nlist_t**)&head, NULL, (nlist_t*)&a);
	print_list(head);
	cl_op.swap((nlist_t**)&head,NULL, (nlist_t*)&c, (nlist_t*)&a);
	print_list(head);
#if 0
	add(&head, &d);
	add(&head, &e);
	add(&head, &f);
	add(&head, &g);
	add(&head, &h);
	printf("Swap 3, 6, expected: 1 2 6 4 5 3 7 8\n");
	swap (&head, &c, &f);
	print_list (head);

	printf("Swap 3, 6, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &c, &f);
	print_list (head);

	search_test(&head, 4);
	search_test(&head, 9);

	printf("Swap 1, 4, expected: 4 2 3 1 5 6 7 8\n");
	swap (&head, &a, &d);
	print_list (head);

	printf("Swap 1, 4, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &a, &d);
	print_list (head);

	printf("Swap 1, 8, expected: 8 2 3 4 5 6 7 1\n");
	swap (&head, &a, &h);
	print_list (head);

	printf("Swap 1, 8, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &a, &h);
	print_list (head);

	printf("Swap 2, 3, expected: 1 3 2 4 5 6 7 8\n");
	swap (&head, &b, &c);
	print_list (head);

	printf("Swap 2, 3, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &b, &c);
	print_list (head);

	printf("Swap 1, 2, expected: 2 1 3 4 5 6 7 8\n");
	swap (&head, &a, &b);
	print_list (head);

	printf("Swap 1, 2, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &a, &b);
	print_list (head);

	printf("Swap 7, 8, expected: 1 2 3 4 5 6 8 7\n");
	swap (&head, &g, &h);
	print_list (head);

	printf("Swap 7, 8, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &g, &h);
	print_list (head);
	printf("---Test of next function. \n");
	print_all_items(head);


	printf ("Del 8, expected: 1 2 3 4 5 6 7\n");
	del (&head, &h);
	print_list (head);

	printf ("Del 7, expected: 1 2 3 4 5 6 \n");
	del (&head, &g);
	print_list (head);
	printf ("Del 6, expected: 1 2 3 4 5\n");
	del (&head, &f);
	print_list (head);

	ac = exist(&head, &g);
	printf ("Is there g? %s\n", ac==1?"Yes":(ac==0?"No":"Something wrong."));
	ac = exist(&head, &e);
	printf ("Is there e? %s\n", ac==1?"Yes":(ac==0?"No":"Something wrong."));

	printf ("Del 5, expected: 1 2 3 4 \n");
	del (&head, &e);
	print_list (head);
	printf ("Del 4, expected: 1 2 3\n");
	del (&head, &d);
	print_list (head);
	printf ("Del 1, expected: 2 3\n");
	del (&head, &a);
	print_list (head);
	printf ("Del 3, expected: 2\n");
	del (&head, &c);
	print_list (head);
	printf("---Test of next function. \n");
	print_all_items(head);
	printf ("Del 2, expected: \n");
	del (&head, &b);
	print_list (head);
#endif
	return 0;
}


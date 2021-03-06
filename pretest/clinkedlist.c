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


struct nlist {
	struct nlist *prev, *next;
	int v;
};

struct nlist* search(struct nlist **head, void *arg, int (*comp)(void*, void*)); 
/* 
   if item exists in target chain, returns 1.
   else if not, returns 0.
   in other case, returns -1.
*/
int exist(struct nlist **head, struct nlist *item)
{
	struct nlist *p = NULL;
	if (!head || !(*head) || !item) {
		return -1;
	}
	p = *head;
	do {
		if (p == item) return 1;
		p = p->next;
		if (p == *head) break;
	} while (p);

	return 0;
}


void add (struct nlist **head, struct nlist *item) 
{
	if (!head || !item) {
		printf("Invalied argument.\n");
		return;
	}

	if (!(*head)) {
		*head = item;
		item->next = item->prev = item;
		return ;
	}
	item->prev = (*head)->prev;
	item->next = *head;
	(*head)->prev->next = item;
	(*head)->prev = item; /* pointer to tail of the linked list. */
}

void del (struct nlist **head, struct nlist *item)
{
	if (*head == item) {
		if (*head == (*head)->prev) {
			*head = NULL;	
			item->next = item->prev = NULL;
			return ;
		}
	//	(*head)->next->prev = (*head)->prev;
	//	(*head)->prev->next = (*head)->next;
		*head = item->next;
	//	item->next = item->prev = NULL;
	//	return ;
	}

	item->prev->next = item->next;
	item->next->prev = item->prev;
	item->next = item->prev = NULL;
	return ;
}

struct nlist * get_next (struct nlist **head, struct nlist **cur)
{
	struct nlist *next_item = NULL;
	if (!head || !(*head) || !cur) {
		return next_item;
	}
	if (!(*cur)){
		next_item = (*head);
		(*cur) = (*head);
		return next_item;
	}
	next_item = (*cur)->next;
	*cur = (*cur)->next;
	if (next_item == (*head)){
		return NULL;
	}
	
	return next_item;
}

void print_item (struct nlist *item)
{
	printf ("%6d | %20p | %20p | %20p\n",
			item->v, item, item->prev, item->next );
}
	
void print_list(struct nlist *head) 
{
	int count = 0;
	struct nlist *tr = head;
	if (!head) {
		printf ("There is no item.\n");
		return ;
	}
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	while (tr) {
		print_item (tr);
		tr = tr->next;
		if(tr == head) break;
	}
}

void print_all_items (struct nlist *head)
{
	struct nlist *next_item = NULL;
	struct nlist *save = NULL;
	if (!head) {
		printf ("There is no item.\n");
		return ;
	}
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	next_item = get_next(&head, &save);
	while (next_item) {
		print_item (next_item);
		next_item = get_next(&head, &save);
	}
}

void swap (struct nlist **head, struct nlist *x, struct nlist *y)
{
	struct nlist **xpn, **xnp, **ypn, **ynp;
	struct nlist *xp , *xn, *yp, *yn;

	if (x == y) return;

	xpn = xnp = ypn = ynp = NULL;
	xp = xn = yp = yn = NULL;

	if (x->prev) { xpn = &(x->prev->next); xp = x->prev; }
	if (x->next) { xnp = &(x->next->prev); xn = x->next; }
	if (y->prev) { ypn = &(y->prev->next); yp = y->prev; }
	if (y->next) { ynp = &(y->next->prev); yn = y->next; }

	if (xn == y || yn == x ){
		struct nlist *prv, *next;
		if (xn == xp) {
			printf("Just two elements.\n");
		/* there are just two elements. so just change the value of head. */
			*head = (*head)->next;
			return;
		}
		printf ("neighborhood\n");
		if (xn == y) {
			prv = x;
			next = y;
		} else {
			prv = y;
			next = x;
		}
		xp = prv->prev;
		xn = prv->next;
		yp = next->prev;
		yn = next->next;

		if (xp) xp->next = next;
		if (yn) yn->prev = prv;
		prv->prev = next;
		next->next = prv;
		prv->next = yn;
		next->prev = xp;
	} else {
		x->prev = yp; 
		y->prev = xp;
		x->next = yn;
		y->next = xn;

		if (xpn) *xpn = y;
		if (xnp) *xnp = y;
		if (ypn) *ypn = x;
		if (ynp) *ynp = x;
	}

	if ( *head == x ) {
		printf("head changed. x->y\n");
		*head = y;
	} else if ( *head == y ) {
		printf("head changed. y->x\n");
		*head = x;
	}
}

struct nlist* search(struct nlist **head, void *arg, int (*comp)(void*, void*))
{
	struct nlist *node = NULL;
	struct nlist *save = NULL;
	int found = 0;
	if (!comp) return NULL;
	node = get_next(head, &save);
	while (node) {
		if (0 == comp((void*)node, arg) ){
			found = 1;
			break;
		}
		node = get_next(head, &save);
	}
	return node;
}

int int_comp (void *li, void *value)
{
	struct nlist *lv = (struct nlist *)li;
	int			val = *(int *)value;	
	if (!lv) return -1;
	return (lv->v) - val;
}

void search_test (struct nlist **head, int search_key)
{
	struct nlist *item = NULL;

	item = search (head, (void*)&search_key, int_comp);
	if (item) {
		printf ("Found. (%d)\n", search_key);
		print_item(item);
	} else {
		printf ("Cannot find. (%d)\n", search_key);
	}
}


int main (int ac, char** av)
{
	struct nlist *head = NULL;
	struct nlist a ={ NULL, NULL, 1 };
	struct nlist b ={ NULL, NULL, 2 };
	struct nlist c ={ NULL, NULL, 3 };
	struct nlist d ={ NULL, NULL, 4 };
	struct nlist e ={ NULL, NULL, 5 };
	struct nlist f ={ NULL, NULL, 6 };
	struct nlist g ={ NULL, NULL, 7 };
	struct nlist h ={ NULL, NULL, 8 };

	add(&head, &a);
	printf ("Address of head: %p\n", head);
	print_list (head);

	printf ("Add 2, expected: 1 2\n");
	add(&head, &b);
	print_list (head);

	printf("Swap 1, 2, expected: 2 1\n");
	swap (&head, &a, &b);
	print_list (head);

	printf ("Swap 1, 2, expected: 1 2\n");
	swap (&head, &a, &b);
	print_list (head);

	printf ("Add 3, expected: 1 2 3\n");
	add(&head, &c);
	print_list(head);

	printf("Swap 1, 2, expected: 2 1 3\n");
	swap (&head, &a, &b);
	print_list (head);

	printf("Swap 1, 2, expected: 1 2 3\n");
	swap (&head, &a, &b);
	print_list (head);

	printf("Del 2, expected: 1 3\n");
	del (&head, &b);
	print_list(head);

	printf("Add 2, expected: 1 3 2\n");
	add (&head, &b);
	print_list(head);

	printf ("Del 1, expected: 3 2\n");
	del (&head, &a);
	print_list(head);

	printf("Add 1, Swap 3, 1,  expected: 1 3 2\n");
	add(&head, &a);
	print_list(head);
	swap(&head, &c, &a);
	print_list(head);

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
	return 0;
}


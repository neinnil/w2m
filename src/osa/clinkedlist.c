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

static void add (struct nlist **head, struct nlist *item) 
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

static void del (struct nlist **head, struct nlist *item)
{
	if (*head == item) {
		if (*head == (*head)->prev) {
			*head = NULL;	
			item->next = item->prev = NULL;
			return ;
		}
		(*head)->next->prev = (*head)->prev;
		(*head)->prev->next = (*head)->next;
		*head = item->next;
		item->next = item->prev = NULL;
		return ;
	}

	item->prev->next = item->next;
	item->next->prev = item->prev;
	item->next = item->prev = NULL;
	return ;
}

#if 0
static void print_item (struct nlist *item)
{
	printf ("%6d | %20p | %20p | %20p\n",
			item->v, item, item->prev, item->next );
}
static void print_list(struct nlist *head) 
{
	int count = 0;
	struct nlist *tr = head;
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	while (tr && count++ < 8) {
		print_item (tr);
		tr = tr->next;
		if(tr == head) break;
	}
}
#endif
static void swap (struct nlist **head, struct nlist *x, struct nlist *y)
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
		/* there are just two elements. I change the value of head. */
			printf("Just two elements.\n");
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

#if 0
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

	printf ("Del 8, expected: 1 2 3 4 5 6 7\n");
	del (&head, &h);
	print_list (head);

	printf ("Del 7, expected: 1 2 3 4 5 6 \n");
	del (&head, &h);
	print_list (head);
	printf ("Del 6, expected: 1 2 3 4 5\n");
	del (&head, &h);
	print_list (head);
	printf ("Del 5, expected: 1 2 3 4 \n");
	del (&head, &h);
	print_list (head);
	printf ("Del 4, expected: 1 2 3\n");
	del (&head, &h);
	print_list (head);
	return 0;
}
#endif

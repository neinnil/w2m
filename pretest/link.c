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


struct ln {
	struct ln *p, *n;
	int v;
};

void add (struct ln **head, struct ln **tail, struct ln *item) 
{
	if (!head || !tail || !item) {
		printf("Invalied argument.\n");
		return;
	}
	if (!(*head) && *tail) {
		printf("broken linked list.\n");
		assert(0);
		return;
	}
	if (!(*head)) {
		*head = item;
		*tail = item;
#if defined(_CIRCULAR_)
		item->n = item->p = item;
#endif
		return ;
	}
	(*tail)->n = item;
	item->p = *tail;
#if defined(_CIRCULAR_)
	item->n = *head;
	(*head)->p = item;
#endif
	*tail = item;
}

void del (struct ln **head, struct ln **tail, struct ln *item)
{
	if (*head == item) {
		*head = item->n;
#if defined(_CIRCULAR_)
		(*head)->p = *tail;
		(*tail)->n = *head;
#else
		(*head)->p = NULL;
#endif
		item->n = item->p = NULL;
		return ;
	}
	if (*tail == item) {
		*tail = item->p;
#if defined(_CIRCULAR_)
		(*head)->p = *tail;
		(*tail)->n = *head;
#else
		(*tail)->n = NULL;
#endif
		item->n = item->p = NULL;
		return ;
	}
	item->p->n = item->n;
	item->n->p = item->p;
	item->n = item->p = NULL;
	return ;
}

void print_item (struct ln *item)
{
	printf ("%6d | %20p | %20p | %20p\n",
			item->v, item, item->p, item->n );
}
	
void print_list(struct ln *head) 
{
	int count = 0;
	struct ln *tr = head;
	printf ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	while (tr && count++ < 8) {
		print_item (tr);
		tr = tr->n;
		if(tr == head) break;
	}
}

void swap (struct ln **head, struct ln **tail, struct ln *x, struct ln *y)
{
	struct ln **xpn, **xnp, **ypn, **ynp;
	struct ln *xp , *xn, *yp, *yn;

	if (x == y) return;

	xpn = xnp = ypn = ynp = NULL;
	xp = xn = yp = yn = NULL;

	if (x->p) { xpn = &(x->p->n); xp = x->p; }
	if (x->n) { xnp = &(x->n->p); xn = x->n; }
	if (y->p) { ypn = &(y->p->n); yp = y->p; }
	if (y->n) { ynp = &(y->n->p); yn = y->n; }

	if (xn == y || yn == x ){
#if defined (_CIRCULAR_)
		if (xn == xp) {
			/* there are just two elements. so just change head and tail. */
			*head = (*head)->n;
			*tail = (*tail)->p;
			return;
		}
#endif
		struct ln *prv, *next;
		printf ("neighborhood\n");
		if (xn == y) {
			prv = x;
			next = y;
		} else {
			prv = y;
			next = x;
		}
		xp = prv->p;
		xn = prv->n;
		yp = next->p;
		yn = next->n;

		if (xp) xp->n = next;
		if (yn) yn->p = prv;
		prv->p = next;
		next->n = prv;
		prv->n = yn;
		next->p = xp;
		
	} else {
		x->p = yp; 
		y->p = xp;
		x->n = yn;
		y->n = xn;

		if (xpn) *xpn = y;
		if (xnp) *xnp = y;
		if (ypn) *ypn = x;
		if (ynp) *ynp = x;
	}

	if ( *head == x ) {
		printf("head changed. x->y\n");
		*head = y;
//		y->p = NULL;
	} else if ( *head == y ) {
		printf("head changed. y->x\n");
		*head = x;
//		x->p = NULL;
	}

	if ( *tail == x )  {
		printf("tail changed. x->y\n");
		*tail = y;
//		y->n = NULL;
	} else if ( *tail == y ) {
		printf("tail changed. y->x\n");
		*tail = x;
//		x->n = NULL;
	}

}


int main (int ac, char** av)
{
	struct ln *head = NULL;
	struct ln *tail = NULL;
	struct ln a ={ NULL, NULL, 1 };
	struct ln b ={ NULL, NULL, 2 };
	struct ln c ={ NULL, NULL, 3 };
	struct ln d ={ NULL, NULL, 4 };
	struct ln e ={ NULL, NULL, 5 };
	struct ln f ={ NULL, NULL, 6 };
	struct ln g ={ NULL, NULL, 7 };
	struct ln h ={ NULL, NULL, 8 };
#if 0 //defined (_CIRCULAR_)
	a.n = a.p = &a;
#endif
	//head = tail = &a;
	add(&head, &tail, &a);
	printf ("Address of head: %p\n", head);
	add(&head, &tail, &b);
	
	print_list (head);

	printf("Swap 1, 2, expected: 2 1\n");
	swap (&head, &tail, &a, &b);
	print_list (head);
	printf ("Swap 1, 2, expected: 1 2\n");
	swap (&head, &tail, &a, &b);
	print_list (head);
	printf ("Add 3, expected: 1 2 3\n");
	add(&head, &tail, &c);
	print_list(head);
	printf("Swap 1, 2, expected: 2 1 3\n");
	swap (&head, &tail, &a, &b);
	print_list (head);

	printf("Swap 1, 2, expected: 1 2 3\n");
	swap (&head, &tail, &a, &b);
	print_list (head);

	printf("Del 2, expected: 1 3\n");
	del (&head, &tail, &b);
	print_list(head);

	printf("Add 2, expected: 1 3 2\n");
	add (&head, &tail, &b);
	print_list(head);

	printf ("Del 1, expected: 3 2\n");
	del (&head, &tail, &a);
	print_list(head);


	printf("Add 1, Swap 3, 1,  expected: 1 3 2\n");
	add(&head, &tail, &a);
	swap(&head, &tail, &c, &a);
	print_list(head);
	add(&head, &tail, &d);
	add(&head, &tail, &e);
	add(&head, &tail, &f);
	add(&head, &tail, &g);
	add(&head, &tail, &h);
	printf("Swap 3, 6, expected: 1 2 6 4 5 3 7 8\n");
	swap (&head, &tail, &c, &f);
	print_list (head);

	printf("Swap 3, 6, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &tail, &c, &f);
	print_list (head);

	printf("Swap 1, 4, expected: 4 2 3 1 5 6 7 8\n");
	swap (&head, &tail, &a, &d);
	print_list (head);

	printf("Swap 1, 4, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &tail, &a, &d);
	print_list (head);

	printf("Swap 1, 8, expected: 8 2 3 4 5 6 7 1\n");
	swap (&head, &tail, &a, &h);
	print_list (head);

	printf("Swap 1, 8, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &tail, &a, &h);
	print_list (head);

	printf("Swap 2, 3, expected: 1 3 2 4 5 6 7 8\n");
	swap (&head, &tail, &b, &c);
	print_list (head);

	printf("Swap 2, 3, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &tail, &b, &c);
	print_list (head);

	printf("Swap 1, 2, expected: 2 1 3 4 5 6 7 8\n");
	swap (&head, &tail, &a, &b);
	print_list (head);

	printf("Swap 1, 2, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &tail, &a, &b);
	print_list (head);

	printf("Swap 7, 8, expected: 1 2 3 4 5 6 8 7\n");
	swap (&head, &tail, &g, &h);
	print_list (head);

	printf("Swap 7, 8, expected: 1 2 3 4 5 6 7 8\n");
	swap (&head, &tail, &g, &h);
	print_list (head);

	return 0;
}

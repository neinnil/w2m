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


struct ln {
	struct ln *p, *n;
	int v;
};

void add (struct ln **head, struct ln **tail, struct ln *item) 
{
	(*tail)->n = item;
	item->p = *tail;
#if defined(_CIRCULAR_)
	item->n = *head;
	(*head)->p = item;
#endif
	*tail = item;
}

void print_item (struct ln *item)
{
	printf ("%6d | %p       | %p      | %p \n",
			item->v, item, item->p, item->n );
}
	
void print_list(struct ln *head) 
{
	int count = 0;
	struct ln *tr = head;
	printf ("value | addr  |  prev | next \n");
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
#if defined (_CIRCULAR_)
	a.n = a.p = &a;
#endif
	head = tail = &a;
	printf ("Address of head: %p\n", head);
	add(&head, &tail, &b);
	add(&head, &tail, &c);
#if 0
	add(&head, &tail, &d);
	add(&head, &tail, &e);
	add(&head, &tail, &f);
	add(&head, &tail, &g);
	add(&head, &tail, &h);
#endif
	
	print_list (head);
#if 1
	printf("Swap 1, 2, expected: 2 1 3\n");
	swap (&head, &tail, &a, &b);
	print_list (head);

	printf("Swap 1, 2, expected: 1 2 3\n");
	swap (&head, &tail, &a, &b);
	print_list (head);

#else
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
#endif
	return 0;
}

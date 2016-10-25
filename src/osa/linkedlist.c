/*
    linkedlist.c 
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

#define _CIRCULAR_ 1

struct ln {
	struct ln *p, *n;
	int v;
};

int add (struct ln **head, struct ln **tail, struct ln *item) 
{
	if (!head || !tail || !item){
		return -1;
	}
	if (!(*head)) {

	}
	(*tail)->n = item;
	item->p = *tail;
#if defined(_CIRCULAR_)
	item->n = *head;
	(*head)->p = item;
#endif
	*tail = item;
	return 0;
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
	}

	if ( *tail == x )  {
		printf("tail changed. x->y\n");
		*tail = y;
	} else if ( *tail == y ) {
		printf("tail changed. y->x\n");
		*tail = x;
	}

}
#if 0
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
#endif

/*
    circular linked list
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
#include "nein/debug.h"

static int  cl_init (struct nlist *p);
static int	cl_exist(struct nlist **head, struct nlist *item);
static int cl_add (struct nlist **head, struct nlist *item);
static int cl_del (struct nlist **head, struct nlist *item);
static void cl_swap (struct nlist **head, struct nlist *x, struct nlist *y);
static struct nlist* cl_get_next (struct nlist **head, struct nlist **cur);
static struct nlist* cl_search(struct nlist **head, void *arg, int (*comp)(void *, void *));


static int cl_init (struct nlist *p)
{
	if (!p) return -1;
	p->next = p->prev = p;
	return 0;
}

/* 
   if item exists in target chain, returns 1.
   else if not, returns 0.
   in other case, returns -1.
*/
static int cl_exist(struct nlist **head, struct nlist *item)
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

static int cl_add (struct nlist **head, struct nlist *item) 
{
	if (!head || !item) {
		NIL_DEBUG("Invalied argument.\n");
		return -1;
	}

	if (!(*head)) {
		*head = item;
		item->next = item->prev = item;
		return 0;
	}
	item->prev = (*head)->prev;
	item->next = *head;
	(*head)->prev->next = item;
	(*head)->prev = item; /* pointer to tail of the linked list. */
	return 0;
}

static int cl_add_2 (nlist_t **head, nlist_t **tail, nlist_t *item)
{
	int ret = 0;
	ret = cl_add(head,item);
	if (0==ret && tail && *head) {
		*tail = (*head)->prev;
	}
	return ret;
}

static int cl_del (struct nlist **head, struct nlist *item)
{
	if (!head || !(*head) || !item) {
		NIL_DEBUG("Invalied argument.\n");
		return -1;
	}
	if (*head == item) {
		if (*head == (*head)->prev) {
			*head = NULL;	
			item->next = item->prev = NULL;
			return 0;
		}
//		(*head)->next->prev = (*head)->prev;
//		(*head)->prev->next = (*head)->next;
		*head = item->next;
//		item->next = item->prev = NULL;
//		return 0;
	}

	item->prev->next = item->next;
	item->next->prev = item->prev;
	item->next = item->prev = NULL;
	return 0;
}

static int cl_del_2 (nlist_t **head, nlist_t **tail,nlist_t *item)
{
	int ret = 0;
	ret = cl_del(head, item);
	if (0==ret && tail && *head) {
		*tail = (*head)->prev;
	}
	return ret;
}

#if 0
static void print_item (struct nlist *item)
{
	NIL_DEBUG ("%6d | %20p | %20p | %20p\n",
			item->v, item, item->prev, item->next );
}
static void print_list(struct nlist *head) 
{
	int count = 0;
	struct nlist *tr = head;
	NIL_DEBUG ("%6s | %-20s | %-20s | %-20s\n","value","addr","prev","next");
	while (tr && count++ < 8) {
		print_item (tr);
		tr = tr->next;
		if(tr == head) break;
	}
}
#endif
static void cl_swap (struct nlist **head, struct nlist *x, struct nlist *y)
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
			NIL_DEBUG("Just two elements.\n");
			*head = (*head)->next;
			return;
		}
		NIL_DEBUG ("neighborhood\n");
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
		NIL_DEBUG("head changed. x->y\n");
		*head = y;
	} else if ( *head == y ) {
		NIL_DEBUG("head changed. y->x\n");
		*head = x;
	}
}

static void cl_swap_2 (nlist_t **head, nlist_t **tail, nlist_t *x, nlist_t *y)
{
	cl_swap(head, x, y);
	if (tail) *tail = (*head)->prev;
}

static	struct nlist * cl_get_next (struct nlist **head, struct nlist **cur)
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

static nlist_t* cl_next_2 (nlist_t** head, nlist_t **tail, nlist_t**cur)
{
	return cl_get_next(head, cur);
}

static struct nlist* cl_search(struct nlist **head, void *arg, int (*comp)(void*, void*))
{
	struct nlist *item = NULL;
	struct nlist *save = NULL;
	int found = 0;
	if (!comp) return NULL;
	item = cl_get_next(head, &save);
	while (item) {
		if (0 == comp((void*)item, arg) ){
			found = 1;
			break;
		}
		item = cl_get_next(head, &save);
	}
	return item;
}

static struct nlist* cl_search_2 (nlist_t **head, nlist_t **tail, void* key, int (*comp)(void*, void*)) 
{
	return cl_search (head, key, comp);
}


struct nlist_op cl_op = {
	.init = cl_init,
	.append = cl_add_2,
	.remove = cl_del_2,
	.exist = cl_exist,
	.swap = cl_swap_2,
	.next = cl_next_2,
	.search = cl_search_2
};

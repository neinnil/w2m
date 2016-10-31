/**
    list.c. 
    Copyright (C) 2016	Park Sangjun <neinnil@gmail.com>

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
**/
#include <stdlib.h>
#include <errno.h>
#include "nein/list.h"

int alloc_list(void **p, size_t nSize)
{
	int ret = 0;
	if (nSize < NLISTSZ){
		nSize = NLISTSZ;
	}
	*p = malloc(nSize);
	ret = init_list((struct nlist*)*p);
	return ret?ret:nSize;
}

int init_list(struct nlist *p)
{
	if (NULL == p){
		return -EINVAL;
	}
	p->next = p->prev = NULL;
	return 0;
}

int add_list (struct nlist **p, struct nlist *adding)
{
	struct nlist *pt = NULL;
	if (NULL == p || NULL == adding){
		return -EINVAL;
	}
	if (*p == NULL) {
		*p = adding;
		return 0;
	}
	pt = *p;
	while(pt->next){
		pt = pt->next;
	}
	pt->next = adding;
	adding->prev = pt;
	return 0;
}

int insert_list (struct nlist **p, struct nlist *n, struct nlist *inserting)
{
	if (NULL == inserting || NULL == p) 
		return -EINVAL;
	if (NULL == n) {
		return add_list (p, inserting);
	} else if (NULL == *p) {
		*p = inserting;
		return add_list (p, n);
	} else {
		(*p)->next = inserting;
		n->prev = inserting;
		inserting->next = n;
		inserting->prev = *p;
	}
	return 0;
}

int delete_item (struct nlist **h, struct nlist *delItem)
{
	struct nlist *p = NULL;
	if (NULL == h || NULL == *h || NULL == delItem) {
		return -EINVAL;
	}
	if (*h == delItem) {
		*h = (*h)->next;
	}
	p = (*h)->next;
	while (p && p != delItem) {
		p = p->next;
	}
	if (p == delItem){
		if (p->prev) p->prev->next = p->next;
		if (p->next) p->next->prev = p->prev;
		p->next = p->prev = NULL;
		return 0;
	}
	return -EEXIST;
}

int delete_listAll ( struct nlist **h, void(*freeFunc)(void *) )
{
	struct nlist *t = NULL, *d = NULL;

	if (NULL == h) 
		return -EINVAL;

	t = *h;
	while (t != NULL) {
		d = t;
		t = t->next;
		if (freeFunc) 
			freeFunc((void*)d);
	}
	*h = NULL;
	return 0;
}

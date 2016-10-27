/**
    list.c. 
    Copyright (C) 2016  Park Sangjun <neinnil@gmail.com>

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
#ifndef _NEIN_LIST_H
#define _NEIN_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

struct nlist {
	struct nlist *prev;
	struct nlist *next;
};

typedef struct nlist nlist_t;

#define NLISTSZ sizeof(struct nlist)

struct nlist_op {
	int (*init)(struct nlist *p);
	int (*append)(struct nlist **h, struct nlist **t, struct nlist *it);
	int (*remove)(struct nlist **h, struct nlist **t, struct nlist *it);
	int (*exist)(struct nlist **h, struct nlist *it);
	void (*swap)(struct nlist **h, struct nlist **t, struct nlist *x, struct nlist *y);
	struct nlist* (*next)(struct nlist **h, struct nlist **t, struct nlist **c);
	struct nlist* (*search)(struct nlist **h, struct nlist **t, void* key, int(*comp)(void *, void*));
};
#if 0
extern int init_list(struct nlist *p);
extern int alloc_list(void **userlist, size_t nSize);
extern int delete_listAll(struct nlist **head);
extern int delete_item(struct nlist** head, struct nlist* item);
extern int add_list (struct nlist **head, struct nlist *adding);
extern int insert_list (struct nlist **p, struct nlist *n, struct nlist *ins);
#endif
extern struct nlist_op cl_op;

#ifdef __cplusplus
}
#endif

#endif /* _NEIN_LIST_H */

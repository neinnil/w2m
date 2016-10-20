/**
    list.c. 
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
**/
#ifndef _NEIN_LIST_H
#define _NEIN_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

struct nlist {
	struct nlist *next;
	struct nlist *prev;
};

#define NLISTSZ sizeof(struct nlist)

extern int init_list(struct nlist *p);
extern int alloc_list(void **userlist, size_t nSize);
extern int delete_listAll(struct nlist **head);
extern int delete_item(struct nlist** head, struct nlist* item);
extern int add_list (struct nlist **p, struct nlist *adding);
extern int insert_list (struct nlist **p, struct nlist *n, struct nlist *ins);

#ifdef __cplusplus
}
#endif

#endif /* _NEIN_LIST_H */

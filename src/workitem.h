#ifndef _WORK_ITEM_H
#define _WORK_ITEM_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _workitem {
	struct nlist	list;
	int				state; /* none (not started), doing, done */	
#define WORK_NOT_STARTED	0
#define WORK_DOING			1
#define WORK_DONE			2
#define WORK_ABORTED		4

	char			*src;
	char			*dst;
	int				isSupported; /* 0: not supported. 1: supported. */
	void			*pcmInfo;
	pthread_mutex_t	*lock;
}


typedef struct _workitem workitem;

extern workitem* alloc_workitem(char *src);
extern void		 free_workitem(workitem *wi);
extern void		 set_state(workitem *wi, int state);
extern int		 get_state(workitem *wi);
extern int		 isSupported(workitem *wi);
extern void		 setSupportedFlag(workitem *wi, int bSupport);
extern int		 setDestination(workitem *wi);

#ifdef __cplusplus
}
#endif

#endif

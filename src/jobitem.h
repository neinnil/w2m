#ifndef _JOB_ITEM_H
#define _JOB_ITEM_H

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _jobitem {
	int				state;	/* none (not started), doing, done */	
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


typedef struct _jobitem jobitem_t;

extern jobitem_t*		alloc_jobitem(char *src);
extern void			free_jobitem(jobitem_t *ji);
extern void			set_state(jobitem_t *ji, int state);
extern int			get_state(jobitem_t *ji);
extern int			isSupported(jobitem_t *ji);
extern void			setSupportedFlag(jobitem_t *ji, int bSupport);
extern int			setDestination(jobitem_t *ji);
extern int			setPcmData (jobitem_t *ji, void *pcmData);
extern int			job_lock (jobitem_t *ji);
extern int			job_unlock (jobitem_t *ji);
extern int			job_trylock (jobitem_t *ji);

#ifdef __cplusplus
}
#endif

#endif

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
#define NOT_SUPPORTED_FILE 0
#define SUPPORTED_FILE 1
	void			*pcmInfo;
	pthread_mutex_t	*lock;
};


typedef struct _jobitem jobitem_t;

extern jobitem_t*	allocJobitem (char *src);
extern void			freeJobitem (jobitem_t *ji);
extern void			setFreeFunction (void (*freeFn)(void *p));
extern void			setStateOfJobitem (jobitem_t *ji, int state);
extern int			getStateOfJobitem (jobitem_t *ji);
extern int			isSupportedJobitem (jobitem_t *ji);
extern void			setSupportedFlag (jobitem_t *ji, int bSupport);
extern int			setDestination (jobitem_t *ji, char *dest);
extern int			setPcmData (jobitem_t *ji, void *pcmData);
extern int			lockJobitem (jobitem_t *ji);
extern int			unlockJobitem (jobitem_t *ji);
extern int			trylockJobitem (jobitem_t *ji);

#ifdef __cplusplus
}
#endif

#endif

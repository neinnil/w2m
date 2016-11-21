/*
	jobitem.c 
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
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "jobitem.h"

jobitem_t*	alloc_jobitem(char *src)
{
	int			rc = 0;
	jobitem_t	*item = NULL;
	if (!src) return NULL;
	item = (jobitem_t*)malloc(sizeof(jobitem_t));
	if (!item) return NULL;
	memset (item, 0x00, sizeof(jobitem_t));
	item->lock =  (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (!item->lock) {
		free (item);
		return NULL;
	}
	if (0!(rc=pthread_mutex_init(item->lock, NULL))) {
		free (item->lock);
		free (item);
		return NULL;
	}
	item->src = strdup ((const char*)src);
	if (!item->src) {
		pthread_mutex_destroy(item->lock);
		free (item->lock);
		free (item);
		return NULL;
	}
	return item;
}

void	free_jobitem(jobitem_t *ji) 
{
	if (ji) {
		int	rc = 0;
		pthread_mutex_lock(ji->lock);

		if (ji->src) free (ji->src);
		if (ji->dst) free (ji->dst);
//		if (ji->pcmInfo) free (pcmInfo);

		pthread_mutex_unlock(ji->lock);
		rc = pthread_mutex_destroy(ji->lock);
		free (ji->lock);
		free (ji);
	}
}

void	set_state(jobitem_t *ji, int state)
{
	if (ji && state > ji->state) {
		ji->state = state;
	}
}

int		get_state(jobitem_t *ji)
{
	if (ji) {
		return ji->state;
	} 
	return -EINVAL;
}

int		isSupported(jobitem_t *ji)
{
	if (ji) {
		return ji->isSupported;
	}
	return -EINVAL;
}

void	setSupportedFlag(jobitem_t *ji, int bSupport)
{
	if (ji) {
		ji->isSupported = bSupport;
	}
}

int		setDestination(jobitem_t *ji, char *dst)
{
	if (ji && dst) {
		ji->dst = strdup ((const char*)dst);
		if (!ji->dst) {
			return -ENOMEM;
		}
		return 0;
	}
	return -EINVAL;
}

int		setPcmData (jobitem_t *ji, void *pcmData)
{
	if (ji && pcmData) {
		ji->pcmInfo = pcmData;
	}
	return -EINVAL;
}

int		job_lock (jobitem_t *ji)
{
	if (ji) {
		return pthread_mutex_lock (ji->lock);
	} 
	return -EINVAL;
}

int		job_unlock (jobitem_t *ji)
{
	if (ji) {
		return pthread_mutex_unlock (ji->lock);
	} 
	return -EINVAL;
}

int		job_trylock (jobitem_t *ji)
{
	if (ji) {
		return pthread_mutex_trylock (ji->lock);
	} 
	return -EINVAL;
}

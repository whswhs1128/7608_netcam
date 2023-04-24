/*!
*****************************************************************************
** \file      $gkprjrefcnt.c
**
** \version	$id: refcnt.c 15-08-04  8月:08:1438655428 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "refcnt.h"

JRefCnt *j_refcnt_alloc(int initval)
{
	JRefCnt *ref = malloc(sizeof(JRefCnt));
	if (!ref)
		return NULL;
	memset(ref, 0, sizeof(JRefCnt));

	ref->refcnt = initval;
	pthread_mutex_init(&ref->lock, NULL);
	pthread_cond_init(&ref->cond, NULL);

	return ref;
}

int j_refcnt_free(JRefCnt *ref)
{
	if (ref) {
		pthread_mutex_destroy(&ref->lock);
		pthread_cond_destroy(&ref->cond);
		free(ref);
	}
	return 0;
}

int j_refcnt_inc(JRefCnt *ref)
{
	pthread_mutex_lock(&ref->lock);
	ref->refcnt++;
	pthread_cond_broadcast(&ref->cond);
	pthread_mutex_unlock(&ref->lock);

	return 0;
}

int j_refcnt_dec(JRefCnt *ref)
{
	pthread_mutex_lock(&ref->lock);
	ref->refcnt--;
	pthread_cond_broadcast(&ref->cond);
	pthread_mutex_unlock(&ref->lock);

	return 0;
}

int j_refcnt_add(JRefCnt *ref, int val)
{
	pthread_mutex_lock(&ref->lock);
	ref->refcnt += val;
	pthread_cond_broadcast(&ref->cond);
	pthread_mutex_unlock(&ref->lock);

	return 0;
}

int j_refcnt_sub(JRefCnt *ref, int val)
{
	pthread_mutex_lock(&ref->lock);
	ref->refcnt -= val;
	pthread_cond_broadcast(&ref->cond);
	pthread_mutex_unlock(&ref->lock);

	return 0;
}

int j_refcnt_get(JRefCnt *ref)
{
	pthread_mutex_lock(&ref->lock);
	int refcnt = ref->refcnt;
	pthread_mutex_unlock(&ref->lock);

	return refcnt;
}

int j_refcnt_wait(JRefCnt *ref, int val)
{
	pthread_mutex_lock(&ref->lock);
	while (ref->refcnt != val)
		pthread_cond_wait(&ref->cond, &ref->lock);
	pthread_mutex_unlock(&ref->lock);

	return 0;
}

int j_refcnt_timedwait(JRefCnt *ref, int val, int ms)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	ts.tv_sec  += ms / 1000;
	ts.tv_nsec += (ms % 1000) * 1000000;
	if (ts.tv_nsec >= 1000000000) {
		ts.tv_sec  += 1;
		ts.tv_nsec %= 1000000000;
	}

	int ret = 0;

	pthread_mutex_lock(&ref->lock);

	while (ref->refcnt != val && ret != ETIMEDOUT)
		ret = pthread_cond_timedwait(&ref->cond, &ref->lock, &ts);

	if (ret == ETIMEDOUT) {
		pthread_mutex_unlock(&ref->lock);
		return 0;
	}

	pthread_mutex_unlock(&ref->lock);

	return 1;
}

/*!
*****************************************************************************
** \file      $gkprjmempool.c
**
** \version	$id: mempool.c 15-08-04  8月:08:1438655388 
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
#include <limits.h>
#include <pthread.h>

#include "mempool.h"

#define max(x, y)	((x) > (y) ? (x) : (y))
#define min(x, y)	((x) < (y) ? (x) : (y))

#define MAX_PREALLOC_OBJS	(1024)

typedef struct Object {
	struct Object *next;
} Object;

struct JMemPool {
	int obj_size;
	int max_objs;

	int num_objs;
	Object *first_object;

	int thread_safe;
	pthread_mutex_t lock;
};

JMemPool *j_mempool_create(int obj_size, int max_objs, int thread_safe)
{
	if (obj_size <= 0)
		return NULL;

	if (max_objs <= 0)
		max_objs = INT_MAX;	/* unlimit */

	JMemPool *m = malloc(sizeof(JMemPool));
	if (!m)
		return NULL;
	memset(m, 0, sizeof(JMemPool));

	m->obj_size    = max(obj_size, sizeof(Object));
	m->max_objs    = max_objs;
	m->thread_safe = thread_safe;

	int num_objs = min(max_objs, MAX_PREALLOC_OBJS);

	if (thread_safe)
		pthread_mutex_init(&m->lock, NULL);

	/*
	 * prealloc some objects
	 */
	int i;
	for (i = 0; i < num_objs; i++) {
		Object *obj = malloc(m->obj_size);
		if (!obj)
			break;
		memset(obj, 0, sizeof(Object));

		obj->next = m->first_object;
		m->first_object = obj;
		m->num_objs++;
	}

	return m;
}

int j_mempool_destroy(JMemPool *m)
{
	if (!m)
		return -1;

	Object *obj, *obj_next;
	for (obj = m->first_object; obj; obj = obj_next) {
		obj_next = obj->next;
		free(obj);
	}

	if (m->thread_safe)
		pthread_mutex_destroy(&m->lock);

	free(m);

	return 0;
}

static void *mempool_alloc(JMemPool *m)
{
	Object *obj = NULL;

	if (m->num_objs > 0) {
		obj = m->first_object;
		m->first_object = obj->next;
		m->num_objs--;
	} else {
		/* no object in pool, allocate a new one */
		obj = malloc(m->obj_size);
	}

	return obj;
}

void *j_mempool_alloc(JMemPool *m)
{
	if (!m)
		return NULL;

	Object *obj = NULL;

	if (m->thread_safe) {
		pthread_mutex_lock(&m->lock);
		obj = mempool_alloc(m);
		pthread_mutex_unlock(&m->lock);
	} else {
		obj = mempool_alloc(m);
	}

	return obj;
}

static int mempool_release(JMemPool *m, void *obj)
{
	if (m->num_objs < m->max_objs) {
		/* append to object list */
		((Object *)obj)->next = m->first_object;
		m->first_object = obj;
		m->num_objs++;
	} else {
		/* too many objects in pool */
		free(obj);
	}

	return 0;
}

int j_mempool_release(JMemPool *m, void *obj)
{
	if (!m || !obj)
		return -1;

	if (m->thread_safe) {
		pthread_mutex_lock(&m->lock);
		mempool_release(m, obj);
		pthread_mutex_unlock(&m->lock);
	} else {
		mempool_release(m, obj);
	}

	return 0;
}

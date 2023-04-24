/*!
*****************************************************************************
** \file      $gkprjqueue.c
**
** \version	$id: queue.c 15-08-04  8月:08:1438655421 
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
#include <pthread.h>

#include "mem.h"
#include "list.h"
#include "queue.h"

struct JQueue {
	int abort;
	JList *list;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};

JQueue *j_queue_new(void)
{
	JQueue *q = j_mallocz(sizeof(JQueue));
	if (!q)
		return NULL;

	q->list = j_list_alloc();
	if (!q->list) {
		j_free(q);
		return NULL;
	}

	pthread_mutex_init(&q->lock, NULL);
	pthread_cond_init(&q->cond, NULL);

	return q;
}

int j_queue_abort(JQueue *q)
{
	pthread_mutex_lock(&q->lock);
	q->abort = 1;
	pthread_cond_broadcast(&q->cond);
	pthread_mutex_unlock(&q->lock);

	return 0;
}

int j_queue_destroy(JQueue *q)
{
	if (!q)
		return -1;

	j_queue_abort(q);

	pthread_mutex_destroy(&q->lock);
	pthread_cond_destroy(&q->cond);

	j_list_free(q->list);
	j_free(q);

	return 0;
}

int j_queue_put(JQueue *q, void *data)
{
	pthread_mutex_lock(&q->lock);
	if (q->abort) {
		pthread_mutex_unlock(&q->lock);
		return -1;
	}

	j_list_prepend(q->list, data);
	pthread_cond_signal(&q->cond);

	pthread_mutex_unlock(&q->lock);

	return 0;
}

void *j_queue_get(JQueue *q, int block)
{
	void *data;

	pthread_mutex_lock(&q->lock);

	while (!q->abort && block && j_list_empty(q->list))
		pthread_cond_wait(&q->cond, &q->lock);

	if (q->abort) {
		pthread_mutex_unlock(&q->lock);
		return NULL;
	}

	data = j_list_remove_last(q->list);

	pthread_mutex_unlock(&q->lock);

	return data;
}

int j_queue_length(JQueue *q)
{
	pthread_mutex_lock(&q->lock);
	int len = j_list_length(q->list);
	pthread_mutex_unlock(&q->lock);

	return len;
}

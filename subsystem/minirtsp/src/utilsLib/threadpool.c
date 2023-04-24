/*!
*****************************************************************************
** \file      $gkprjthreadpool.c
**
** \version	$id: threadpool.c 15-08-04  8月:08:1438655446 
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
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "mem.h"
#include "queue.h"
#include "threadpool.h"
#include "mempool.h"
//#include "sdk_sys.h"


struct JThreadPool {
	int max_jobs;
	JQueue *jobq;

	int nb_workers;
	pthread_t *workers;

	JMemPool *mempool;

	int abort_request;
};

typedef struct Job {
	int (*func)(void *ctx);
	void *ctx;
} Job;

static void *worker(void *arg)
{
    sdk_sys_thread_set_name("worker");
	JThreadPool *tp = arg;
	if (!tp)
		return NULL;

	while (!tp->abort_request) {
		Job *j = j_queue_get(tp->jobq, 1);
		if (j) {
			j->func(j->ctx);
			j_mempool_release(tp->mempool, j);
		}
	}

	return 0;
}

JThreadPool *j_thread_pool_new(int max_workers, int max_jobs)
{
	JThreadPool *tp = j_mallocz(sizeof(JThreadPool));
	if (!tp)
		return NULL;

	tp->max_jobs = max_jobs;

	tp->jobq = j_queue_new();
	if (!tp->jobq)
		goto fail;

	tp->workers = j_calloc(max_workers, sizeof(pthread_t));
	if (!tp->workers)
		goto fail;

	tp->mempool = j_mempool_create(sizeof(Job), tp->max_jobs, 1);
	if (!tp->mempool)
		goto fail;

	int i;
	for (i = 0; i < max_workers; i++) {
		if (pthread_create(&tp->workers[tp->nb_workers], NULL, worker, tp) != 0)
			goto fail;

		tp->nb_workers++;
	}

	return tp;

fail:
	j_thread_pool_destroy(tp);
	return NULL;
}

int j_thread_pool_destroy(JThreadPool *tp)
{
	tp->abort_request = 1;

	if (tp->jobq)
		j_queue_abort(tp->jobq);

	if (tp->workers) {
		int i;
		for (i = 0; i < tp->nb_workers; i++)
			pthread_join(tp->workers[i], NULL);
		j_free(tp->workers);
	}

	if (tp->jobq) {
		Job *j;
		while ((j = j_queue_get(tp->jobq, 0)))	/* nonblock get */
			j_free(j);
		j_queue_destroy(tp->jobq);
	}

	if (tp->mempool) {
		j_mempool_destroy(tp->mempool);
	}

	j_free(tp);

	return 0;
}

int j_thread_pool_add_job(JThreadPool *tp, int (*func)(void *ctx), void *ctx)
{
	if (tp->max_jobs > 0) {
		if (j_queue_length(tp->jobq) >= tp->max_jobs)
			return -1;
	}

	Job *j = j_mempool_alloc(tp->mempool);
	if (!j)
		return -1;
	memset(j, 0, sizeof(Job));

	j->func = func;
	j->ctx  = ctx;

	j_queue_put(tp->jobq, j);

	return 0;
}

int j_thread_pool_stat(JThreadPool *tp, int *nb_workers, int *nb_jobs)
{
	if (!tp)
		return -1;

	if (nb_workers)
		*nb_workers = tp->nb_workers;

	if (nb_jobs)
		*nb_jobs = j_queue_length(tp->jobq);

	return 0;
}

//#define TEST

#ifdef TEST
static void do_print(void *ctx)
{
	long id = (long)ctx;
	printf("%03ld: hello, world!\n", id);
}

int main(void)
{
	int i;
	for (i = 0; i < 10000; i++) {
		printf("round: %d\n", i);

		JThreadPool *tp = j_thread_pool_new(32);
		if (!tp)
			return -1;

		long i;
		for (i = 0; i < 1000; i++)
			j_thread_pool_add_job(tp, do_print, (void *)i);

		sleep(1);

		j_thread_pool_destroy(tp);
	}

	sleep(100);

	return 0;
}
#endif

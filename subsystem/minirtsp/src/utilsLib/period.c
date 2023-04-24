/*!
*****************************************************************************
** \file      $gkprjperiod.c
**
** \version	$id: period.c 15-08-04  8月:08:1438655402 
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
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "mem.h"
#include "list.h"
#include "timer.h"
#include "period.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

struct JPeriodTask {
	char name[64];

	int delay_ms;
	int period_ms;

	int (*action)(void *ctx);
	void *ctx;

	int64_t expose;
};

struct JPeriod {
	int period;
	int abort_request;

	JList *alltasks;

	pthread_t tid;
	pthread_mutex_t lock;
};

static int do_action(JPeriod *per)
{
	JListNode *n, *n_next;
	int64_t now = j_timer_now();

	for (n = j_list_first(per->alltasks); n; n = n_next) {
		n_next = j_list_next(per->alltasks, n);

		JPeriodTask *tsk = n->data;

		if (tsk->expose > now)
			continue;

		if (tsk->action)
			tsk->action(tsk->ctx);

		tsk->expose += tsk->period_ms;	/* reload it */
	}

	return 0;
}

static void *period_routinue(void *arg)
{
    sdk_sys_thread_set_name("period_routinue");
	JPeriod *per = arg;
	if (!per)
		return NULL;

	JTimer *tmr = j_timer_create();
	if (!tmr)
		return NULL;

	j_timer_set(tmr, per->period);

	j_timer_start(tmr);

	while (!per->abort_request) {
		pthread_mutex_lock(&per->lock);
		do_action(per);
		pthread_mutex_unlock(&per->lock);

		j_timer_wait(tmr);
	}

	j_timer_destroy(tmr);
	
	return NULL;
}

JPeriod *j_period_alloc(int period_ms)
{
	if (period_ms <= 0)
		return NULL;

	JPeriod *per = j_mallocz(sizeof(JPeriod));
	if (!per)
		return NULL;

	per->period = period_ms;
	per->alltasks = j_list_alloc();
	if (!per->alltasks) {
		j_free(per);
		return NULL;
	}

	pthread_mutex_init(&per->lock, NULL);

	if (pthread_create(&per->tid, NULL, period_routinue, per) != 0) {
		pthread_mutex_destroy(&per->lock);
		j_free(per);
		return NULL;
	}

	return per;
}

void j_period_free(JPeriod *per)
{
	if (per) {
		per->abort_request = 1;
		pthread_join(per->tid, NULL);

		JListNode *n;
		while ((n = j_list_first(per->alltasks)) != NULL) {
			JPeriodTask *tsk = n->data;
			j_list_remove(per->alltasks, tsk);
			j_free(tsk);
		}

		j_list_free(per->alltasks);
		pthread_mutex_destroy(&per->lock);
		j_free(per);
	}
}

JPeriodTask *j_period_add(JPeriod *per, int delay_ms, int period_ms, int (*action)(void *ctx), void *ctx)
{
	if (!per || !per->alltasks)
		return NULL;

	JPeriodTask *tsk = j_mallocz(sizeof(JPeriodTask));
	if (!tsk)
		return NULL;

	tsk->delay_ms  = delay_ms;
	tsk->period_ms = period_ms;
	tsk->action    = action;
	tsk->ctx       = ctx;

	tsk->expose = j_timer_now() + tsk->delay_ms;

	pthread_mutex_lock(&per->lock);
	j_list_append(per->alltasks, tsk);
	pthread_mutex_unlock(&per->lock);

	return tsk;
}

int j_period_del(JPeriod *per, JPeriodTask *tsk)
{
	if (!per || !per->alltasks || !tsk)
		return -1;

	pthread_mutex_lock(&per->lock);
	j_list_remove(per->alltasks, tsk);
	pthread_mutex_unlock(&per->lock);

	j_free(tsk);

	return 0;
}

//#define TEST

#ifdef TEST
static int tsk_action(void *ctx)
{
	int id = (int)ctx;

	printf("%s: id = %d\n", __func__, id);

	return 0;
}

int main(void)
{
	JPeriod *per = j_period_alloc(10);
	if (!per)
		return -1;

	JPeriodTask *tsk1 = j_period_add(per, 1000, 10000, tsk_action, (void *)1);
	JPeriodTask *tsk2 = j_period_add(per, 1000, 30000, tsk_action, (void *)2);

	sleep(100);

	j_period_del(per, tsk1);
	j_period_del(per, tsk2);

	j_period_free(per);

	return 0;
}
#endif

/*!
*****************************************************************************
** \file      $gkprjtimer.c
**
** \version	$id: timer.c 15-08-04  8月:08:1438655453 
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
#include <time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "timer.h"

struct JTimer {
	int state;
	int64_t orig;				/* in microseconds */
	int64_t posix_timer_time;		/* in microseconds */
	int64_t interval;			/* in microseconds */
};

static int64_t j_gettime_ms(void)
{
	struct timespec tp;
	if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0)
		return -1;
	return (int64_t)tp.tv_sec * 1000 + (int64_t)tp.tv_nsec / 1000000;
}

int64_t j_timer_now(void)
{
	return j_gettime_ms();
}

JTimer *j_timer_create(void)
{
	JTimer *tmr = malloc(sizeof(JTimer));
	if (!tmr)
		return NULL;
	memset(tmr, 0, sizeof(JTimer));

	return tmr;
}

int j_timer_destroy(JTimer *tmr)
{
	free(tmr);
	return 0;
}

void j_timer_start(JTimer *tmr)
{
	tmr->state = J_TIMER_RUNNING;
	tmr->orig = j_gettime_ms();
	tmr->posix_timer_time = tmr->interval;
}

int j_timer_wait(JTimer *tmr)
{
	int64_t diff, time;
	struct timeval tv;

	while (1) {
		time = j_gettime_ms() - tmr->orig;
		diff = tmr->posix_timer_time - time;
		if (diff < 0) {
//			printf("Must catchup %lli miliseconds.\n", diff);
			break;
		}

//		printf("Must wait %lli miliseconds.\n", diff);
		tv.tv_sec  = diff / 1000;
		tv.tv_usec = diff % 1000 * 1000;
		select(0, NULL, NULL, NULL, &tv);
	}

	tmr->posix_timer_time += tmr->interval;

	return 0;
}

void j_timer_reset(JTimer *tmr)
{
	tmr->state = J_TIMER_STOPPED;
	tmr->orig = j_gettime_ms();
	tmr->posix_timer_time = 0;
}

void j_timer_set(JTimer *tmr, int interval_ms)
{
	tmr->interval = interval_ms;
}

//#define TEST

#ifdef TEST
int main(void)
{
	JTimer *tmr;
	tmr = j_timer_create();

	struct timeval tv;
	tv.tv_sec  = 0;
	tv.tv_usec = 40000;	/* 40 microseconds */
	j_timer_set(tmr, &tv);

	j_timer_start(tmr);
	j_timer_wait(tmr);
	j_timer_destroy(tmr);

	return 0;
}
#endif	/* TEST */

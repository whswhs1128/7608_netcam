/*!
*****************************************************************************
** \file      $gkprjflag.c
**
** \version	$id: flag.c 15-08-04  8月:08:1438655363 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "flag.h"

JFlag *j_flag_alloc(void)
{
	JFlag *f = malloc(sizeof(JFlag));
	if (!f)
		return NULL;
	memset(f, 0, sizeof(JFlag));

	pthread_mutex_init(&f->lock, NULL);
	pthread_cond_init(&f->cond, NULL);

	return f;
}

int j_flag_free(JFlag *f)
{
	pthread_mutex_destroy(&f->lock);
	pthread_cond_destroy(&f->cond);
	free(f);

	return 0;
}

int j_flag_set(JFlag *f, int bit)
{
	if (bit >= sizeof(f->mask) * 8)
		return -1;

	pthread_mutex_lock(&f->lock);
	f->mask |= (1 << bit);
	pthread_cond_broadcast(&f->cond);
	pthread_mutex_unlock(&f->lock);

	return 0;
}

int j_flag_clr(JFlag *f, int bit)
{
	if (bit >= sizeof(f->mask) * 8)
		return -1;

	pthread_mutex_lock(&f->lock);
	f->mask &= ~(1 << bit);
	pthread_cond_broadcast(&f->cond);
	pthread_mutex_unlock(&f->lock);

	return 0;
}

int j_flag_get(JFlag *f, int bit)
{
	if (bit >= sizeof(f->mask) * 8)
		return -1;

	pthread_mutex_lock(&f->lock);
	int val = !!(f->mask & (1 << bit));
	pthread_mutex_unlock(&f->lock);

	return val;
}

int j_flag_wait(JFlag *f, int mask)
{
	pthread_mutex_lock(&f->lock);
	while (f->mask != mask)
		pthread_cond_wait(&f->cond, &f->lock);
	pthread_mutex_unlock(&f->lock);

	return 0;
}

//#define TEST

#ifdef TEST
int main(void)
{
	return 0;
}
#endif

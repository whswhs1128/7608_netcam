/*!
*****************************************************************************
** \file      $gkprjrefcnt.h
**
** \version	$id: refcnt.h 15-08-04  8月:08:1438655554 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __UTILS_REFCNT_H__
#define __UTILS_REFCNT_H__

typedef struct JRefCnt {
	int refcnt;

	pthread_mutex_t lock;
	pthread_cond_t cond;
} JRefCnt;

JRefCnt *j_refcnt_alloc(int initval);
int j_refcnt_free(JRefCnt *ref);

int j_refcnt_inc(JRefCnt *ref);
int j_refcnt_dec(JRefCnt *ref);

int j_refcnt_add(JRefCnt *ref, int val);
int j_refcnt_sub(JRefCnt *ref, int val);

int j_refcnt_get(JRefCnt *ref);

int j_refcnt_wait(JRefCnt *ref, int val);
int j_refcnt_timedwait(JRefCnt *ref, int val, int ms);

#endif /* __UTILS_REFCNT_H__ */

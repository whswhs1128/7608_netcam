/*!
*****************************************************************************
** \file      $gkprjthreadpool.h
**
** \version	$id: threadpool.h 15-08-04  8月:08:1438655570 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

typedef struct JThreadPool JThreadPool;

JThreadPool *j_thread_pool_new(int num_workers, int max_jobs);
int j_thread_pool_destroy(JThreadPool *tp);

int j_thread_pool_add_job(JThreadPool *tp, int (*func)(void *ctx), void *ctx);

int j_thread_pool_stat(JThreadPool *tp, int *nb_workers, int *nb_jobs);

#endif /* __THREADPOOL_H__ */

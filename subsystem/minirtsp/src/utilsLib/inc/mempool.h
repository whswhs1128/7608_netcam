/*!
*****************************************************************************
** \file      $gkprjmempool.h
**
** \version	$id: mempool.h 15-08-04  8月:08:1438655526 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

typedef struct JMemPool JMemPool;

JMemPool *j_mempool_create(int obj_size, int max_objs, int with_lock);
int j_mempool_destroy(JMemPool *m);

void *j_mempool_alloc(JMemPool *m);
int j_mempool_release(JMemPool *m, void *obj);

#endif	/* __MEMPOOL_H__ */

/*!
*****************************************************************************
** \file      $gkprjpool.h
**
** \version	$id: pool.h 15-08-04  8月:08:1438655544 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __POOL_H__
#define __POOL_H__

#include <unistd.h>
#include <sys/uio.h>

#include "fifo.h"

typedef struct JPool {
	int size;
	JFifo *fifo;

	pthread_cond_t cond;
	pthread_mutex_t lock;
} JPool;

JPool *j_pool_alloc(int size);
void j_pool_free(JPool *pool);

ssize_t j_pool_read (JPool *pool, void *buf, size_t buf_size, int milliseconds);
ssize_t j_pool_write(JPool *pool, void *buf, size_t buf_size, int milliseconds);

ssize_t j_pool_readv (JPool *pool, struct iovec *iov, int iovcnt, int milliseconds);
ssize_t j_pool_writev(JPool *pool, struct iovec *iov, int iovcnt, int milliseconds);

#endif /* __POOL_H__ */

/*!
*****************************************************************************
** \file      $gkprjpool.c
**
** \version	$id: pool.c 15-08-04  8月:08:1438655416 
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
#include <errno.h>

#include <sys/time.h>
#include <sys/uio.h>

#include "fifo.h"
#include "pool.h"

#define min(x, y)	((x) < (y) ? (x) : (y))
#define max(x, y)	((x) > (y) ? (x) : (y))

typedef struct FrameHeader {
	uint32_t length;
} FrameHeader;

static struct timespec time_after(int milliseconds)
{
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);

	int64_t nsec = tp.tv_nsec + (int64_t)milliseconds * 1000000;

	tp.tv_sec  += nsec / 1000000000;
	tp.tv_nsec  = nsec % 1000000000;

	return tp;
}

static int iovec_size(const struct iovec *iov, int iovcnt)
{
	int i;
	int len = 0;

	for (i = 0; i < iovcnt; i++)
		len += iov[i].iov_len;

	return len;
}

JPool *j_pool_alloc(int size)
{
	JPool *pool = malloc(sizeof(JPool));
	if (!pool)
		return NULL;
	memset(pool, 0, sizeof(JPool));

	pool->size = size;
	pool->fifo = j_fifo_alloc(size);
	if (!pool->fifo) {
		free(pool);
		return NULL;
	}

	pthread_cond_init(&pool->cond, NULL);
	pthread_mutex_init(&pool->lock, NULL);

	return pool;
}

void j_pool_free(JPool *pool)
{
	if (pool) {
		pthread_cond_destroy(&pool->cond);
		pthread_mutex_destroy(&pool->lock);

		j_fifo_free(pool->fifo);
		free(pool);
	}
}

static ssize_t pool_readv(JPool *pool, struct iovec *iov, int iovcnt)
{
	FrameHeader h;
	j_fifo_generic_read(pool->fifo, &h, sizeof(FrameHeader), NULL);

	int left  = h.length;
	int space = iovec_size(iov, iovcnt);

	int i;
	for (i = 0; i < iovcnt; i++) {
		struct iovec *v = &iov[i];

		int size = min(left, v->iov_len);
		j_fifo_generic_read(pool->fifo, v->iov_base, size, NULL);

		left  -= size;
		space -= size;

		if (!left || !space)
			break;
	}

	return h.length - left;
}

static ssize_t pool_writev(JPool *pool, struct iovec *iov, int iovcnt)
{
	int size = iovec_size(iov, iovcnt);

	/* write frame header */
	FrameHeader h = { .length = size };
	j_fifo_generic_write(pool->fifo, &h, sizeof(FrameHeader), NULL);

	/* write frame data   */
	int i;
	for (i = 0; i < iovcnt; i++) {
		struct iovec *v = &iov[i];
		j_fifo_generic_write(pool->fifo, v->iov_base, v->iov_len, NULL);
	}

	return size;
}

ssize_t j_pool_readv(JPool *pool, struct iovec *iov, int iovcnt, int milliseconds)
{
	struct timespec tp = time_after(milliseconds);

	pthread_mutex_lock(&pool->lock);

	while (j_fifo_size(pool->fifo) == 0) {
		int ret = pthread_cond_timedwait(&pool->cond, &pool->lock, &tp);
		if (ret != 0) {
			pthread_mutex_unlock(&pool->lock);
			return (ret == ETIMEDOUT) ? 0 : -1;
		}
	}

	ssize_t size = pool_readv(pool, iov, iovcnt);
	if (size > 0)
		pthread_cond_signal(&pool->cond);

	pthread_mutex_unlock(&pool->lock);

	return size;
}

ssize_t j_pool_writev(JPool *pool, struct iovec *iov, int iovcnt, int milliseconds)
{
	if (!pool || !iov || !iovcnt)
		return -1;

	int total = iovec_size(iov, iovcnt) + sizeof(FrameHeader);
	if (total > pool->size)
		return -1;

	struct timespec tp = time_after(milliseconds);

	pthread_mutex_lock(&pool->lock);

	while (j_fifo_space(pool->fifo) < total) {
		int ret = pthread_cond_timedwait(&pool->cond, &pool->lock, &tp);
		if (ret != 0) {
			pthread_mutex_unlock(&pool->lock);
			return (ret == ETIMEDOUT) ? 0 : -1;
		}
	}

	ssize_t size = pool_writev(pool, iov, iovcnt);
	if (size > 0)
		pthread_cond_signal(&pool->cond);

	pthread_mutex_unlock(&pool->lock);

	return size;
}

ssize_t j_pool_read(JPool *pool, void *buf, size_t size, int milliseconds)
{
	struct iovec iov = {.iov_base = buf, .iov_len = size};
	return j_pool_readv(pool, &iov, 1, milliseconds);
}

ssize_t j_pool_write(JPool *pool, void *buf, size_t size, int milliseconds)
{
	struct iovec iov = {.iov_base = buf, .iov_len = size};
	return j_pool_writev(pool, &iov, 1, milliseconds);
}

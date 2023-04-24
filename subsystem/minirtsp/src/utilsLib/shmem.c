/*!
*****************************************************************************
** \file      $gkprjshmem.c
**
** \version	$id: shmem.c 15-08-04  8月:08:1438655435 
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
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "shmem.h"

struct JShmem {
	char name[128];

	sem_t rsem;
	sem_t wsem;

	int shutdown;

	int total;

	int datsiz;
	uint8_t data[0];
};

JShmem *j_shmem_create(const char *name, int size)
{
	int fd = shm_open(name, O_CREAT | O_RDWR, 0644);
	if (fd < 0) {
		perror("shm_open");
		return NULL;
	}

	int totalsize = sizeof(JShmem) + size;

	ftruncate(fd, totalsize);

	JShmem *f = mmap(NULL, totalsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!f) {
		close(fd);
		shm_unlink(name);
		return NULL;
	}
	memset(f, 0, sizeof(JShmem));

	close(fd);	/* close here */

	strncpy(f->name, name, sizeof(f->name) - 1);

	sem_init(&f->rsem, 1, 0);
	sem_init(&f->wsem, 1, 1);

	f->total = size;

//	printf("create shmem %s success\n", name);

	return f;
}

JShmem *j_shmem_connect(const char *name, int size)
{
	int fd = shm_open(name, O_RDWR, 0644);
	if (fd < 0) {
		perror("shm_open");
		return NULL;
	}

	int totalsize = sizeof(JShmem) + size;

	ftruncate(fd, totalsize);

	JShmem *f = mmap(NULL, totalsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (!f) {
		close(fd);
		shm_unlink(name);
		return NULL;
	}

	close(fd);	/* close here */

//	printf("connected to %s success\n", name);

	return f;
}

int j_shmem_destroy(JShmem *f)
{
	f->shutdown = 1;
	char name[128] = {0};
	strncpy(name, f->name, sizeof(name) - 1);

	int totalsize = sizeof(JShmem) + f->total;

	munmap(f, totalsize);
	shm_unlink(name);

	return 0;
}

#define min(x, y)	((x) < (y) ? (x) : (y))

ssize_t j_shmem_read(JShmem *f, void *buf, size_t size, ShmemFunc func, void *ctx, int milliseconds)
{
	if (!f || (!buf && !func))
		return -1;

	int len;

	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);

	t.tv_sec  += milliseconds / 1000;
	t.tv_nsec += milliseconds % 1000 * 1000000;

	if (t.tv_nsec >= 1000000000) {
		t.tv_sec  += 1;
		t.tv_nsec -= 1000000000;
	}

	if (sem_timedwait(&f->rsem, &t) < 0) {
		if (f->shutdown) {
			errno = ESHUTDOWN;
			return -1;
		}
		if (errno == ETIMEDOUT)
			return -1;
		return -1;
	}

	if (f->shutdown) {
		errno = ESHUTDOWN;
		return -1;
	}

	if (func) {
		len = func(f->data, f->datsiz, ctx);
		if (len < 0) {
			f->shutdown = 1;	/* shut it down */
			sem_post(&f->wsem);	/* wakeup the writer */
			return -1;
		}
	} else {
		len = min(f->datsiz, size);
		memcpy(buf, f->data, len);
	}

	sem_post(&f->wsem);

	return len;
}

ssize_t j_shmem_write(JShmem *f, void *buf, size_t size, ShmemFunc func, void *ctx, int milliseconds)
{
	if (!f || (!buf && !func))
		return -1;

	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);

	t.tv_sec  += milliseconds / 1000;
	t.tv_nsec += milliseconds % 1000 * 1000000;

	if (t.tv_nsec >= 1000000000) {
		t.tv_sec  += 1;
		t.tv_nsec -= 1000000000;
	}

	if (sem_timedwait(&f->wsem, &t) < 0) {
		if (f->shutdown) {
			errno = ESHUTDOWN;
			return -1;
		}
		if (errno == ETIMEDOUT)
			return -1;
		return -1;
	}

	if (f->shutdown) {
		errno = ESHUTDOWN;
		return -1;
	}

	if (func) {
		f->datsiz = func(f->data, f->total, ctx);
		if (f->datsiz < 0) {
			f->shutdown = 1;
			sem_post(&f->rsem);	/* wakeup reader */
			return -1;
		} else if (f->datsiz == 0) {
			sem_post(&f->wsem);	/* reset as writable */
			errno = ENODATA;
			return 0;
		}
	} else {
		f->datsiz = min(size, f->total);
		memcpy(f->data, buf, f->datsiz);
	}

	sem_post(&f->rsem);

	return f->datsiz;
}

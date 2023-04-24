/*!
*****************************************************************************
** \file      $gkprjshmem.h
**
** \version	$id: shmem.h 15-08-04  8月:08:1438655562 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __SHMEM_H__
#define __SHMEM_H__

#include <stdint.h>
#include <semaphore.h>

typedef struct JShmem JShmem;

typedef int (*ShmemFunc)(void *buf, size_t size, void *ctx);

JShmem *j_shmem_create (const char *name, int size);
JShmem *j_shmem_connect(const char *name, int size);

int j_shmem_destroy(JShmem *f);

ssize_t j_shmem_read (JShmem *f, void *buf, size_t size, ShmemFunc func, void *ctx, int milliseconds);
ssize_t j_shmem_write(JShmem *f, void *buf, size_t size, ShmemFunc func, void *ctx, int milliseconds);

#endif /* __SHMEM_H__ */

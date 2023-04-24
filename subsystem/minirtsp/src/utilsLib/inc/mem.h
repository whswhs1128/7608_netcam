/*!
*****************************************************************************
** \file      $gkprjmem.h
**
** \version	$id: mem.h 15-08-04  8月:08:1438655518 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __MEM_H__
#define __MEM_H__

void *j_malloc(size_t size);
void *j_mallocz(size_t size);
void j_free(void *ptr);

void *j_calloc(size_t nmemb, size_t size);
void *j_memdup(void *src, size_t size);

#endif /* __MEM_H__ */

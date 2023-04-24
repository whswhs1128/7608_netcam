/*!
*****************************************************************************
** \file      $gkprjfdevent.h
**
** \version	$id: fdevent.h 15-08-04  8月:08:1438655486 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __FDEVENT_H_
#define __FDEVENT_H_

#include <stdint.h>

typedef struct JFdEvent JFdEvent;

typedef int (*fdevent_cb)(int fd, uint32_t event, void *ctx);

JFdEvent *j_fdevent_create(void);
int j_fdevent_destroy(JFdEvent *evbase);

int j_fdevent_mod(JFdEvent *evbase, int fd, fdevent_cb cb, void *ctx, uint32_t events);
int j_fdevent_add(JFdEvent *evbase, int fd, fdevent_cb cb, void *ctx, uint32_t events);
int j_fdevent_del(JFdEvent *evbase, int fd);

#endif

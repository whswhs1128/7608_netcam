/*!
*****************************************************************************
** \file      $gkprjqueue.h
**
** \version	$id: queue.h 15-08-04  8月:08:1438655549 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct JQueue JQueue;

JQueue *j_queue_new(void);
int j_queue_destroy(JQueue *q);
int j_queue_abort(JQueue *q);

int j_queue_put(JQueue *q, void *data);
void *j_queue_get(JQueue *q, int block);
int j_queue_length(JQueue *q);

#endif /* __QUEUE_H__ */

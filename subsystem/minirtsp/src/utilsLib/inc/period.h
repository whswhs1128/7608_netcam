/*!
*****************************************************************************
** \file      $gkprjperiod.h
**
** \version	$id: period.h 15-08-04  8月:08:1438655538 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __PERIOD_H__
#define __PERIOD_H__

typedef struct JPeriodTask JPeriodTask;
typedef struct JPeriod JPeriod;

JPeriod *j_period_alloc(int period_ms);
void j_period_free(JPeriod *per);

JPeriodTask *j_period_add(JPeriod *per, int delay_ms, int period_ms, int (*action)(void *ctx), void *ctx);
int j_period_del(JPeriod *per, JPeriodTask *tsk);

#endif /* __PERIOD_H__ */

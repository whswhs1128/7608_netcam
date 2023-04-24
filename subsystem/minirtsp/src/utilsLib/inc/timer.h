/*!
*****************************************************************************
** \file      $gkprjtimer.h
**
** \version	$id: timer.h 15-08-04  8月:08:1438655577 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __TIMER_H_
#define __TIMER_H_

#define J_TIMER_STOPPED 0
#define J_TIMER_RUNNING 1

typedef struct JTimer JTimer;

int64_t j_timer_now(void);
JTimer *j_timer_create(void);
int j_timer_destroy(JTimer *tmr);
void j_timer_start(JTimer *tmr);
int j_timer_wait(JTimer *tmr);
void j_timer_reset(JTimer *tmr);
void j_timer_set(JTimer *tmr, int interval_ms);

#endif /* __AVTIMER_H_ */

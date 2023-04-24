/*!
*****************************************************************************
** \file      $gkprjapplications/netcam/src/watchdog/netcam_watchdog.c
**
** \version	$id: applications/netcam/src/watchdog/netcam_watchdog.c 15-08-29  8月:08:1440827281
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
#include <pthread.h>

#include "sdk_api.h"
#include "sdk_watchdog.h"
#include "netcam_api.h"
//#include "work_queue.h"

static pthread_mutex_t watchdog_lock = PTHREAD_MUTEX_INITIALIZER;

static void watchdog_enter_lock(void)
{
    pthread_mutex_lock(&watchdog_lock);
}

static void watchdog_leave_lock(void)
{
    pthread_mutex_unlock(&watchdog_lock);
}

void netcam_watchdog_feed(void )
{
    watchdog_enter_lock();
    sdk_watchdog_feed();
    watchdog_leave_lock();
}

int netcam_watchdog_init(int time_s)
{
    int ret = 0;
    ret = sdk_watchdog_init(time_s);
	// watchdog check in main thread
    //netcam_timer_add_task(netcam_watchdog_feed, 15*NETCAM_TIMER_ONE_SEC, true, false);
    return ret;
}

int netcam_watchdog_exit(void)
{
    return sdk_watchdog_exit();
}

int netcam_watchdog_set_timeout(int time_s)
{
	return sdk_watchdog_set_timeout(time_s);
}


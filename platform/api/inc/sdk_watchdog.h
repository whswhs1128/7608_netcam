/*!
*****************************************************************************
** \file      $gkprjsdk/inc/sdk_watchdog.h
**
** \version	$id: sdk/inc/sdk_watchdog.h 15-08-29  8月:08:1440828115
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef _SDK_WATCHDOG_H_
#define _SDK_WATCHDOG_H_
#ifdef __cplusplus
extern "C" {
#endif

int sdk_watchdog_init(int timeout_sec);

int sdk_watchdog_control(int enable);

int sdk_watchdog_exit(void);

int sdk_watchdog_feed(void);

int sdk_watchdog_set_timeout(int timeout_sec);

int sdk_watchdog_get_timeout();

#ifdef __cplusplus
}
#endif

#endif



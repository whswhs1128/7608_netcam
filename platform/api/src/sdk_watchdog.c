/*!
*****************************************************************************
** \file      $gkprjsdk/GK7101/gk7101_watchdog.c
**
** \version	$id: sdk/GK7101/gk7101_watchdog.c 15-08-29  8月:08:1440829625
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h> // must be after than sys/ioctl.h

#include "sdk_watchdog.h"
#include "sdk_debug.h"

#define watchdog_dev "/dev/watchdog"

static int wgid = 0;

#define ENABLE_WATCH_DOG    1
int sdk_watchdog_init(int timeout_s)
{
    int ret  = 0;
	#if ENABLE_WATCH_DOG
    if(wgid)
        return 0;
    wgid = open(watchdog_dev, O_RDWR);
    if(wgid < 0){
        wgid = 0;
        LOG_ERR("Open watch dog error:%s",strerror(errno));
        ret = -1;
    }
    else
    {
        ret = sdk_watchdog_set_timeout(timeout_s);
        LOG_INFO("get watchdog timeout:%d \r\n", sdk_watchdog_get_timeout());
        ret |= sdk_watchdog_control(1);

    }
	#endif
    return ret;
}
int sdk_watchdog_exit(void)
{
    #if ENABLE_WATCH_DOG
    LOG_INFO("sdk_watchdog_exit, wgid:%d\n", wgid);
	if(wgid){
		sdk_watchdog_control(0);
		close(wgid);
		wgid = 0;
	}
	#endif
    return 0;
}
int sdk_watchdog_feed(void)
{
	#if ENABLE_WATCH_DOG
	int ret = -1;
	int arg = 0;
	if(wgid > 0){
        //LOG_INFO("watchdog keeplive...");
		ret = ioctl(wgid, WDIOC_KEEPALIVE, &arg);
		if(0 == ret){
			return 0;
		}
	}
    else
    {
        LOG_ERR("watchdog feed error,ret:%d\n",ret);
    }
	return -1;
	#else
	return 0;
	#endif
}

int sdk_watchdog_set_timeout(int timeout_sec)
{
	
#if ENABLE_WATCH_DOG
	int ret = -1;

	if(!(timeout_sec >= 0 && timeout_sec <= 2862)){
		LOG_ERR("Timeout Out of Range!");
		return -1;
	}
	if(wgid > 0){
		ret = ioctl(wgid, WDIOC_SETTIMEOUT, &timeout_sec);
		if(0 == ret){
			return 0;
		}
	}
    else
    {
        LOG_ERR("watchdog WDIOC_SETTIMEOUT error,ret:%d\n",ret);
    }
	return -1;
	#else
	return 0;
	#endif

}

int sdk_watchdog_get_timeout()
{
    #if ENABLE_WATCH_DOG
	int ret = 0;
	int timeout_s = 0;
	if(wgid > 0){
		ret = ioctl(wgid, WDIOC_GETTIMEOUT, &timeout_s);
		if(0 == ret){
			return timeout_s;
		}
	}
	LOG_ERR("Get Timeout Failed!");
	return -1;
	#else
	return 0;
	#endif
}

int sdk_watchdog_control(int flag)
{
    #if ENABLE_WATCH_DOG
    int ret = 0;
    int status = flag ? WDIOS_ENABLECARD : WDIOS_DISABLECARD;
    if(wgid > 0){
        ret = ioctl(wgid, WDIOC_SETOPTIONS, &status);
        if(0 == ret){
            return 0;
        }
    }
    LOG_ERR("Watch dog %s Failed!", flag ? "Enable" : "Disable");
    return -1;
	#else
	return 0;
	#endif
}


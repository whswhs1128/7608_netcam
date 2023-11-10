/*!
*****************************************************************************
** FileName     : cfg_gb28181.h
**
** Description  : config for gb28181.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_GB28181_H__
#define _GK_CFG_GB28181_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***       audio                 ***/
/***********************************/


typedef struct {
	char ServerIP[50];
	int  ServerPort;
	char ServerUID[50];
	char ServerPwd[50];
	char ServerDomain[50];
	int  DevicePort;
	char DeviceUID[50];
	int  Expire;
	int  DevHBCycle;
	int  DevHBOutTimes;
	char GBWarnEnable;
	char AlarmID[50];
} GK_NET_GB28181_CFG, *lPGK_NET_GB28181_CFG;
extern void  GB28181Cfg_init(void);
extern int GB28181CfgSave();
extern int GB28181CfgLoad();
extern void GB28181CfgPrint();
extern int GB28181LoadDefValue();
cJSON *GB28181CfgGetJsonSting();
int GB28181CfgCheckSave(GK_NET_GB28181_CFG gb28181Cfg);


#define GB28181_CFG_FILE "gk_gb28181_cfg.cjson"

extern GK_NET_GB28181_CFG runGB28181Cfg;
extern GK_CFG_MAP gb28181Map[];

#ifdef __cplusplus
}
#endif
#endif


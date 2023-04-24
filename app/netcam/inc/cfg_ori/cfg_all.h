/*!
*****************************************************************************
** FileName     : cfg_all.h
**
** Description  : config for all modules.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_ALL_H__
#define _GK_CFG_ALL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_audio.h"
#include "cfg_image.h"
#include "cfg_video.h"
#include "cfg_system.h"
#include "cfg_network.h"
#include "cfg_channel.h"
#include "cfg_md.h"
#include "cfg_snap.h"
#include "cfg_record.h"
#include "cfg_alarm.h"
#include "cfg_ptz.h"
#include "cfg_user.h"
#include "cfg_store.h"
#include "cfg_pwm.h"
#ifdef MODULE_SUPPORT_GB28181
#include "cfg_gb28181.h"
#endif
#include "sdk_cfg.h"

#if defined MODULE_SUPPORT_TUTK || defined MODULE_SUPPORT_GOOLINK
#include "cfg_p2p.h"
#endif

typedef enum {
    SYSTEM_PARAM_ID = 0,
    AUDIO_PARAM_ID,
    VIDEO_PARAM_ID,
    IMAGE_PARAM_ID,
    CHANNEL_PARAM_ID,
    NETWORK_PARAM_ID,
    MD_PARAM_ID,
    SNAP_PARAM_ID,
    RECORD_PARAM_ID,
    ALARM_PARAM_ID,
    PTZ_PARAM_ID,
    USER_PARAM_ID,
    PTZ_DEC_PARAM_ID,
    PTZ_PRESET_PARAM_ID,
    PTZ_CRUISE_PARAM_ID,
    GB28181_PARAM_ID,
    P2P_PARAM_ID,
    PARAM_MAX_ID,
} PARAM_ID;


typedef struct privete_cfg_param_t
{
	int CfgId;
	int (*privete_cfg_save)(void);
	int (*privete_cfg_Load)(void);
	int (*privete_cfg_Load_def_value)(void);
	int (*privete_cfg_get_param)(void *dest);
	int (*privete_cfg_set_param)(void *src);
	void *PriveteCfgAddr;
	int PriveteCfgSize;
}PRIVETE_CFG_T;

typedef struct student{
	PRIVETE_CFG_T privete_cfg;
	struct student *next;
} CFG_LIST_T;

extern int CfgInit();
extern int CfgUnInit();
extern int CfgSaveAll();
extern int CfgLoadAll();
extern int CfgLoadDefValueAll();

int get_param(int id, void *dest);
int set_param(int id, void *src);

int schedule_time_to_slice(GK_SCHEDTIME  *s_time, SDK_U32 *s_slice);
extern void reg_privete_cfg_save(int(*privete_func)());
extern void reg_privete_cfg_Load(int(*privete_func)());
extern void reg_privete_cfg_Load_def_value(int(*privete_func)());
extern void reg_privete_cfg_get_param(int(*privete_func)());
extern void reg_privete_cfg_set_param(int(*privete_func)());


extern pthread_mutex_t g_cfg_write_mutex;

#ifdef __cplusplus
}
#endif
#endif

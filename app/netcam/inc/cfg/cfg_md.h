/*!
*****************************************************************************
** FileName     : cfg_md.h
**
** Description  : config for motion detect.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_MD_H__
#define _GK_CFG_MD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"
#include "cfg_alarm.h"

/***********************************/
/***    motion detect            ***/
/***********************************/
#define GK_NET_MD_GRID_ROW 15
#define GK_NET_MD_GRID_COLUMN 22

#pragma pack(4)

typedef struct {
    SDK_S32 row;       //15
    SDK_S32 column;    //22
    SDK_S8  granularity[GK_NET_MD_GRID_ROW * GK_NET_MD_GRID_COLUMN + 1];
    SDK_S8  res;
} GK_NET_MD_GRID;

typedef struct {
    SDK_S32 enable;
    SDK_S32 x;
    SDK_S32 y;
    SDK_S32 width;
    SDK_S32 height;
} GK_NET_MD_REGION;

typedef struct {
    SDK_S8              screenShotStatus;
    SDK_S8              smartStatus;
    SDK_S8              smartType;    
    SDK_S8              defenceStatus;
    SDK_S8              defenceWeek[7][256];
    SDK_S8              areaEventStatus;
    SDK_S8              areaSensitive;
    SDK_S8              areaAction;
    SDK_S8              areaShowArea;
    SDK_S8              areaScreenShotStatus;
    SDK_S8              areaSmartStatus;
    SDK_S8              areaSmartType;    
    SDK_S8              areaDefenceStatus;
    SDK_S8              areaGroupResolution[12];    
    SDK_S8              areaGroupPoints[128];    
    SDK_S8              areaDefenceWeek[7][256];
    SDK_S8              areaPresetPosition; 

} GK_NET_MD_MOJING_CFG;

typedef struct {
    SDK_S32             channel;
    SDK_S32             enable;        //是否进行布防
    SDK_S32             sensitive;     //灵敏度 取值0 - 100, 越小越灵敏*/
    SDK_S32             compensation;  // 0 , 1
    SDK_S32             detectionType; // 0 grid, 1 region
    GK_NET_MD_GRID      mdGrid;
    GK_NET_MD_REGION    mdRegion[4];
    GK_HANDLE_EXCEPTION handle;   
    SDK_S32             schedule_mode;     //1(默认值)按布防时间scheduleTime, scheduleSlice   0全时段检测
    GK_SCHEDTIME        scheduleTime[7][4];  /*该通道的布防时间*/
    SDK_U32             scheduleSlice[7][3]; /*该通道的布防时间片段*/
    GK_NET_MD_MOJING_CFG mojingMdCfg;
} GK_NET_MD_CFG, *LPGK_NET_MD_CFG;
#pragma pack()

extern int MdCfgSave();
extern int MdCfgLoad();
extern void MdCfgPrint();
extern int MdCfgLoadDefValue();

#define MD_CFG_FILE "gk_md_cfg.cjson"

extern GK_NET_MD_CFG runMdCfg;
extern GK_CFG_MAP mdMap[];

#ifdef __cplusplus
}
#endif
#endif


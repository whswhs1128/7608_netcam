/*!
*****************************************************************************
** FileName     : cfg_store.h
**
** Description  : after reset, config is still stored.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
    
#ifndef _GK_CFG_STORE_H__
#define _GK_CFG_STORE_H__
    
#ifdef __cplusplus
extern "C"
{
#endif
    
#include "cfg_common.h"

/***********************************/
/***         store              ***/
/***********************************/
typedef struct {
    SDK_S32      enable;            /*是否生效*/
    SDK_S32      timezone;          /* -720, +720  时区 分钟 */
    SDK_S32      languageType;      /* 0: 中文; 1 英文 2 ... */
    
} GK_NET_STORE_CFG, *LPGK_NET_STORE_CFG;

extern int StoreCfgSave();
extern int StoreCfgLoad();
extern void StoreCfgPrint();
extern int StoreCfgLoadDefValue();

#define STORE_CFG_FILE "gk_cfg.store"

extern GK_NET_STORE_CFG runStoreCfg;
extern GK_CFG_MAP storeMap[];

#ifdef __cplusplus
}
#endif
#endif


/*!
*****************************************************************************
** FileName     : cfg_store.c
**
** Description  : store config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#include "cfg_store.h"

GK_NET_STORE_CFG runStoreCfg;


GK_CFG_MAP storeMap[] = {
    {"enable",       &runStoreCfg.enable,       GK_CFG_DATA_TYPE_S32,   "0",   1, 0, 1,   NULL},
    {"timeZone",     &runStoreCfg.timezone,     GK_CFG_DATA_TYPE_S32,   "480", 1, -720, 720,  NULL},
    {"languageType", &runStoreCfg.languageType, GK_CFG_DATA_TYPE_S32,   "0",   1, 0, 100,   NULL},

    {NULL,},   
};



void StoreCfgPrint()
{
    printf("********** Store *********\n");
    CfgPrintMap(storeMap);
    printf("********** Store *********\n\n");
}

int StoreCfgSave()
{
    int ret = CfgSave(STORE_CFG_FILE, "store", storeMap);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", STORE_CFG_FILE);
        return -1;
    }

    return 0;
}

int StoreCfgLoad()
{
    int ret = CfgLoad(STORE_CFG_FILE, "store", storeMap);
    if (ret != 0) {
        PRINT_ERR("CfgLoad %s error.", STORE_CFG_FILE);
        return -1;
    }

    return 0;
}

int StoreCfgLoadDefValue()
{
    CfgLoadDefValue(storeMap);

    return 0;
}



/*!
*****************************************************************************
** FileName     : cfg_common.h
**
** Description  : common api of config module.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_COMMON_H__
#define _GK_CFG_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "cJSON.h"
#define CFG_DIR     "/opt/custom/cfg/"
#define CUSTOM_DIR  "/opt/custom/"
#define DEFCFG_DIR  "/usr/local/defcfg/"

typedef enum {
	GK_CFG_DATA_TYPE_U32 = 0,
	GK_CFG_DATA_TYPE_U16,
	GK_CFG_DATA_TYPE_U8,
	GK_CFG_DATA_TYPE_S32,
	GK_CFG_DATA_TYPE_S16,
	GK_CFG_DATA_TYPE_S8,
	GK_CFG_DATA_TYPE_FLOAT,
	//GK_CFG_DATA_TYPE_DOUBLE,
	GK_CFG_DATA_TYPE_STRING,
	GK_CFG_DATA_TYPE_STIME,
	GK_CFG_DATA_TYPE_SLICE,
} GK_CFG_DATA_TYPE;

typedef struct
{
    char* 					stringName;
    void* 					dataAddress;
    GK_CFG_DATA_TYPE		dataType;
    char*                   defaultValue;

    int mode;     // 0-readonly, 1 write and read
    double min; //下限
    double max; //上限

    char* description;
} GK_CFG_MAP;

/*系统时间*/
typedef struct {
    SDK_U16  year;
    SDK_U16  month;
    SDK_U16  day;
    SDK_U16  hour;
    SDK_U16  minute;
    SDK_U16  second;
} GK_NET_TIME, *LPGK_NET_TIME;

typedef struct {
    //开始时间
    SDK_S32 startHour;
    SDK_S32 startMin;
    //结束时间
    SDK_S32 stopHour;
    SDK_S32 stopMin;
} GK_SCHEDTIME;


extern char *CfgReadJsonFile(const char *filename);
extern int CfgWriteToFile(const char *filename, const char *data);
extern char *CfgReadFromFile(const char *filename);

extern int CfgPrintMap(GK_CFG_MAP *mapArray);
extern int CfgSave(const char *filename, const char *str, GK_CFG_MAP *mapArray);
extern int CfgLoad(const char *filename, const char *str, GK_CFG_MAP *mapArray);
extern int CfgLoadDefValue(GK_CFG_MAP *mapArray);

extern cJSON *CfgDataToCjsonByMap(GK_CFG_MAP *mapArray);
extern int CfgCjsonToDataByMap(GK_CFG_MAP *mapArray, cJSON *json);

extern int CfgAddCjson(cJSON *root, const char *str, GK_CFG_MAP *mapArray);
extern int CfgParseCjson(cJSON *root, const char *str, GK_CFG_MAP *mapArray);

extern int CfgGetDefByName(GK_CFG_MAP *mapArray, const char *item_name, void *value);

extern int is_in_schedule_slice(SDK_U32 *slice);
extern int is_in_schedule_timepoint(GK_SCHEDTIME *time_point);
extern int is_in_schedule_timepoint_report(GK_SCHEDTIME *time_point, int* index);
extern int is_in_schedule_timepoint_report_with_cnt(GK_SCHEDTIME *time_point, int scheldueCnt, int* index);

#define MAX_STR_LEN_16 16
#define MAX_STR_LEN_20 20
#define MAX_STR_LEN_32 32
#define MAX_STR_LEN_64 64
#define MAX_STR_LEN_128 128
#define MAX_STR_LEN_256 256

#define MAX_SYSTEM_STR_SIZE 32
#define MAX_TIME_STR_SIZE 32
#define MAX_URL_STR_SIZE 100

#define USE_DEFAULT_PARAM 0

#define ACTION_TIME_SEGNUM 4

#ifdef __cplusplus
}
#endif
#endif /* _GK_CFG_COMMON_H__ */


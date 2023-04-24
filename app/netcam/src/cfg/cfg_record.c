/*!
*****************************************************************************
** FileName     : cfg_record.c
**
** Description  : record config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#include "cfg_record.h"

GK_NET_RECORD_CFG runRecordCfg;

#if USE_DEFAULT_PARAM
GK_CFG_MAP recordMap[] = {
	{"sdCardFormatFlag", &runRecordCfg.sdCardFormatFlag,	  GK_CFG_DATA_TYPE_S32,   "1",   1, 0, 1,	NULL},
    {"enable",           &runRecordCfg.enable,                GK_CFG_DATA_TYPE_S32,   "1",   1, 0, 1,   NULL},
    {"stream_no",        &runRecordCfg.stream_no,             GK_CFG_DATA_TYPE_S32,   "0",   1, 0, 3,   "0-3"},
    {"recordMode",       &runRecordCfg.recordMode,            GK_CFG_DATA_TYPE_S32,   "3",   1, 0, 3,   NULL},
    {"scheduleTime",     &(runRecordCfg.scheduleTime[0][0]),  GK_CFG_DATA_TYPE_STIME, "0",   1, 0, 0,  NULL},
    {"scheduleSlice",    &(runRecordCfg.scheduleSlice[0][0]), GK_CFG_DATA_TYPE_SLICE, "4294967295", 1, 0, 0, NULL},

    {"preRecordTime",  &runRecordCfg.preRecordTime,        GK_CFG_DATA_TYPE_S32,   "2",   1, 0, 10,  NULL},
    {"audioRecEnable", &runRecordCfg.audioRecEnable,       GK_CFG_DATA_TYPE_S32,   "1",   1, 0, 1,   NULL},
    {"recAudioType",   &runRecordCfg.recAudioType,         GK_CFG_DATA_TYPE_S32,   "2",   1, 0, 3,   NULL},
    {"recordLen",      &runRecordCfg.recordLen,            GK_CFG_DATA_TYPE_S32,   "5",  1, 1, 1440, NULL},
    {"recycleRecord",  &runRecordCfg.recycleRecord,        GK_CFG_DATA_TYPE_S32,   "1",   1, 0, 1,   NULL},
    {"mojingRec0",              &(runRecordCfg.mojingRecTime[0]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec1",              &(runRecordCfg.mojingRecTime[1]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec2",              &(runRecordCfg.mojingRecTime[2]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec3",              &(runRecordCfg.mojingRecTime[3]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec4",              &(runRecordCfg.mojingRecTime[4]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec5",              &(runRecordCfg.mojingRecTime[5]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec6",              &(runRecordCfg.mojingRecTime[6]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {NULL,},
};


#else
GK_CFG_MAP recordMap[] = {
    {"enable",           &runRecordCfg.enable,                  },
    {"stream_no",        &runRecordCfg.stream_no,               },
    {"recordMode",       &runRecordCfg.recordMode,              },
    {"scheduleTime",     &(runRecordCfg.scheduleTime[0][0]),    },
    {"scheduleSlice",    &(runRecordCfg.scheduleSlice[0][0]),   },

    {"preRecordTime",  &runRecordCfg.preRecordTime,             },
    {"audioRecEnable", &runRecordCfg.audioRecEnable,            },
    {"recAudioType",   &runRecordCfg.recAudioType,              },
    {"recordLen",      &runRecordCfg.recordLen,                 },
    {"recycleRecord",  &runRecordCfg.recycleRecord,             },
	{"sdCardFormatFlag", &runRecordCfg.sdCardFormatFlag,		GK_CFG_DATA_TYPE_S32,	"1",   1, 0, 1,	NULL},
    {"mojingRec0",              &(runRecordCfg.mojingRecTime[0]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec1",              &(runRecordCfg.mojingRecTime[1]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec2",              &(runRecordCfg.mojingRecTime[2]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec3",              &(runRecordCfg.mojingRecTime[3]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec4",              &(runRecordCfg.mojingRecTime[4]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec5",              &(runRecordCfg.mojingRecTime[5]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {"mojingRec6",              &(runRecordCfg.mojingRecTime[6]),            GK_CFG_DATA_TYPE_STRING,   "0:0-23:59", 1, 1, 256, NULL},
    {NULL,},
};
#endif

void RecordCfgPrint()
{
    printf("********** Record *********\n");
    CfgPrintMap(recordMap);
    printf("********** Record *********\n\n");
}

int RecordCfgSave()
{
    int ret = CfgSave(RECORD_CFG_FILE, "record", recordMap);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", RECORD_CFG_FILE);
        return -1;
    }

    return 0;
}

int RecordCfgLoad()
{
    int ret = CfgLoad(RECORD_CFG_FILE, "record", recordMap);
    if (ret != 0) {
        PRINT_ERR("CfgLoad %s error.", RECORD_CFG_FILE);
        return -1;
    }

    return 0;
}

int RecordCfgLoadDefValue()
{
    CfgLoadDefValue(recordMap);

    return 0;
}


/*!
*****************************************************************************
** FileName     : cfg_snap.c
**
** Description  : snap config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#include "cfg_snap.h"
#include "sdk_sys.h"
extern int copy_file(char *src_name, char *des_name);


GK_NET_SNAP_CFG runSnapCfg;

#if USE_DEFAULT_PARAM
GK_CFG_MAP snapTimerMap[] = {
    {"enable",            &runSnapCfg.timer_snap.enable,               GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, NULL},
    {"scheduleTime",      &(runSnapCfg.timer_snap.scheduleTime[0][0]), GK_CFG_DATA_TYPE_STIME, "0",   "rw", 0, 28, NULL},
    {"interval",          &runSnapCfg.timer_snap.interval,             GK_CFG_DATA_TYPE_S32,   "10000",   "rw", 0, 1000000, NULL},
    {"nums",              &runSnapCfg.timer_snap.nums,                 GK_CFG_DATA_TYPE_S32,   "1",   "rw", 1, 30,   NULL},
    {"pictureQuality",    &runSnapCfg.timer_snap.pictureQuality,       GK_CFG_DATA_TYPE_S32,   "2",   "rw", 0, 4, NULL},
    {"imageSize",         &runSnapCfg.timer_snap.imageSize,            GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 4, "0: 1080p 1: 720p 2 D1 3：CIF，4：QCIF"},
    {"snapShotImageType", &(runSnapCfg.timer_snap.snapShotImageType),  GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, "0 JPEG；1 bmp"},
    {"storagerMode",      &(runSnapCfg.timer_snap.storagerMode),       GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 2, "0 local；1 ftp; 2 local|ftp"},
    {"channelID",         &(runSnapCfg.timer_snap.channelID),          GK_CFG_DATA_TYPE_S32,   "3",   "rw", 3, 3, "channel id for snapshort"},
    {NULL,},
};

GK_CFG_MAP snapEventMap[] = {
	{"enable",            &runSnapCfg.event_snap.enable,               GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, NULL},
    {"interval",          &runSnapCfg.event_snap.interval,             GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1000, NULL},
    {"nums",              &runSnapCfg.event_snap.nums,                 GK_CFG_DATA_TYPE_S32,   "1",   "rw", 1, 30,   NULL},
    {"pictureQuality",    &runSnapCfg.event_snap.pictureQuality,       GK_CFG_DATA_TYPE_S32,   "100", "rw", 0, 100, NULL},
    {"imageSize",         &runSnapCfg.event_snap.imageSize,            GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 4, "0: 1080p 1: 720p 2 D1 3：CIF，4：QCIF"},
    {"snapShotImageType", &(runSnapCfg.event_snap.snapShotImageType),  GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 1, "0 JPEG；1 bmp"},
    {"storagerMode",      &(runSnapCfg.event_snap.storagerMode),       GK_CFG_DATA_TYPE_S32,   "0",   "rw", 0, 2, "0 local；1 ftp; 2 local|ftp"},
    {"channelID",         &(runSnapCfg.event_snap.channelID),          GK_CFG_DATA_TYPE_S32,   "3",   "rw", 3, 3, "channel id for snapshort"},
    {NULL,},   
};
#else
GK_CFG_MAP snapTimerMap[] = {
    {"enable",            &runSnapCfg.timer_snap.enable,                    },
    {"scheduleTime",      &(runSnapCfg.timer_snap.scheduleTime[0][0]),      },
    {"interval",          &runSnapCfg.timer_snap.interval,                  },
    {"nums",              &runSnapCfg.timer_snap.nums,                      },
    {"pictureQuality",    &runSnapCfg.timer_snap.pictureQuality,            },
    {"imageSize",         &runSnapCfg.timer_snap.imageSize,                 },
    {"snapShotImageType", &(runSnapCfg.timer_snap.snapShotImageType),       },
    {"storagerMode",      &(runSnapCfg.timer_snap.storagerMode),            },
    {"channelID",         &(runSnapCfg.timer_snap.channelID),               },
    {NULL,},
};

GK_CFG_MAP snapEventMap[] = {
	{"enable",            &runSnapCfg.event_snap.enable,                    },
    {"interval",          &runSnapCfg.event_snap.interval,                  },
    {"nums",              &runSnapCfg.event_snap.nums,                      },
    {"pictureQuality",    &runSnapCfg.event_snap.pictureQuality,            },
    {"imageSize",         &runSnapCfg.event_snap.imageSize,                 },
    {"snapShotImageType", &(runSnapCfg.event_snap.snapShotImageType),       },
    {"storagerMode",      &(runSnapCfg.event_snap.storagerMode),            },
    {"channelID",         &(runSnapCfg.event_snap.channelID),               },
    {NULL,},
};
#endif
void SnapCfgPrint()
{
    printf("**********Timer Snap *********\n");
    CfgPrintMap(snapTimerMap);
    printf("**********Timer Snap *********\n\n");

	printf("**********Event Snap *********\n");
    CfgPrintMap(snapEventMap);
    printf("**********Event Snap *********\n\n");
}

int SnapCfgSave()
{
	cJSON *root;
    char *out;

	root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "timerSnap", snapTimerMap);
    CfgAddCjson(root, "eventSnap", snapEventMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(SNAP_CFG_FILE, out);
    if (ret != 0) {
        PRINT_ERR("CfgWriteToFile %s error.", SNAP_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);
    return 0;
}

int SnapCfgLoad()
{
    int load_def_config = 0;
    char *data = NULL;

start_load:
    data = CfgReadFromFile(SNAP_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        PRINT_INFO("load %s error, so to load default cfg param.\n", SNAP_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }

    CfgParseCjson(json, "timerSnap", snapTimerMap);
    CfgParseCjson(json, "eventSnap", snapEventMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    if(!load_def_config)
    {
        load_def_config = 1;
        char src_name[64], des_name[64];
        memset(src_name, 0, sizeof(src_name));
        snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, SNAP_CFG_FILE);
        memset(des_name, 0, sizeof(des_name));
        snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, SNAP_CFG_FILE);
        if(!copy_file(src_name, des_name))
            goto start_load;
    }
    SnapCfgLoadDefValue();
    SnapCfgSave();
    return 0;
}

int SnapCfgLoadDefValue()
{
    CfgLoadDefValue(snapTimerMap);
	CfgLoadDefValue(snapEventMap);
    return 0;
}


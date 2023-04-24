/*!
*****************************************************************************
** FileName     : cfg_alarm.c
**
** Description  : alarm config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_alarm.h"
#include "sdk_sys.h"

extern int copy_file(char *src_name, char *des_name);

GK_NET_ALARM_CFG runAlarmCfg;

#if USE_DEFAULT_PARAM
GK_CFG_MAP alarmInMap[] = {
    {"alarmIn.channel",      &(runAlarmCfg.alarmIn.channel),            GK_CFG_DATA_TYPE_S32,   "0", 1, 0, 3,    NULL},
    {"alarmIn.alarmName",    &(runAlarmCfg.alarmIn.alarmInName),        GK_CFG_DATA_TYPE_STRING,"alarm", 1, 1, MAX_STR_LEN_32, NULL},
    {"alarmIn.defaultState", &(runAlarmCfg.alarmIn.defaultState),       GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, "0-low 1-high"},
    {"alarmIn.activeState",  &(runAlarmCfg.alarmIn.activeState),        GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, "0-low 1-high"},

    {"alarmIn.handle.intervalTime", &(runAlarmCfg.alarmIn.handle.intervalTime), GK_CFG_DATA_TYPE_U32,  "5", 1, 1, 120,  NULL},
    {"alarmIn.handle.is_snap",      &(runAlarmCfg.alarmIn.handle.is_snap), 		GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1,   NULL},
	{"alarmIn.handle.snapNum",	  	&(runAlarmCfg.alarmIn.handle.snapNum), 		GK_CFG_DATA_TYPE_U32,  "1", 1, 1, 100, NULL},
    {"alarmIn.handle.is_beep",      &(runAlarmCfg.alarmIn.handle.is_beep), 		GK_CFG_DATA_TYPE_U8,   "0", 1, 0, 1,   NULL},
	{"alarmIn.handle.beepTime",	  	&(runAlarmCfg.alarmIn.handle.beepTime),		GK_CFG_DATA_TYPE_U32,  "3", 1, 1, 600, NULL},
    {"alarmIn.scheduleTime", 		&(runAlarmCfg.alarmIn.scheduleTime[0][0]),  GK_CFG_DATA_TYPE_STIME, "0", 1, 0, 28, NULL},
    {NULL,},
};

GK_CFG_MAP alarmOutMap[] = {
    {"alarmOut.channel",       &(runAlarmCfg.alarmOut.channel),            GK_CFG_DATA_TYPE_S32,   "0", 1, 0, 3,    NULL},
    {"alarmOut.alarmOutName",  &(runAlarmCfg.alarmOut.alarmOutName),       GK_CFG_DATA_TYPE_S8,    "0", 1, 1, MAX_STR_LEN_32, NULL},
    {"alarmOut.defaultState",  &(runAlarmCfg.alarmOut.defaultState),       GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, "0-low 1-high"},
    {"alarmOut.activeState",   &(runAlarmCfg.alarmOut.activeState),        GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, "0-low 1-high"},
    {"alarmOut.powerOnState",  &(runAlarmCfg.alarmOut.powerOnState),       GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, "0-pulse 1-continuous"},
    {"alarmOut.pulseDuration", &(runAlarmCfg.alarmOut.pulseDuration),      GK_CFG_DATA_TYPE_S32,   "5000", 1, 1000, 10000, NULL},
    {"alarmOut.scheduleTime",  &(runAlarmCfg.alarmOut.scheduleTime[0][0]), GK_CFG_DATA_TYPE_STIME, "0", 1, 0, 28, NULL},
    {NULL,},
};

GK_CFG_MAP alarmCfgMap[] = {
	{"gpio_enable",           &(runAlarmCfg.cfg.gpio_enable),       GK_CFG_DATA_TYPE_S8,   "0", 1, 0, 1,    NULL},
	{"in_level",              &(runAlarmCfg.cfg.in_level),          GK_CFG_DATA_TYPE_S8,   "0", 1, 0, 1,    NULL},
	{"out_level",             &(runAlarmCfg.cfg.out_level),         GK_CFG_DATA_TYPE_S8,   "0", 1, 0, 1,    NULL},
	{"io_linkage",            &(runAlarmCfg.cfg.io_linkage),        GK_CFG_DATA_TYPE_S8,   "0", 1, 0, 1,    NULL},
	
    {"upload_interval",       &(runAlarmCfg.cfg.upload_interval),   GK_CFG_DATA_TYPE_S8,   "0", 1, 0, 255,    NULL},
    {"alarmbymail",           &(runAlarmCfg.cfg.alarmbymail),       GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, NULL},
    {"schedule_enable",       &(runAlarmCfg.cfg.schedule_enable),   GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, NULL},
    {"snapshot_enable",       &(runAlarmCfg.cfg.snapshot_enable),   GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, NULL},
    {"recorder_enable",       &(runAlarmCfg.cfg.recorder_enable),   GK_CFG_DATA_TYPE_S8,    "0", 1, 0, 1, NULL},
    {"move_perset",           &(runAlarmCfg.cfg.move_perset),       GK_CFG_DATA_TYPE_S8,    "1", 1, 1, 16, NULL},
    {"alarmMute",             &(runAlarmCfg.cfg.alarmMute),         GK_CFG_DATA_TYPE_S8,      "0", 1, 0, 1, NULL},
    {NULL,},
};

GK_CFG_MAP alarmMojingMap[] = {
	{"mdGbStatus",             &(runAlarmCfg.alarmInfo[0].gbStatus),       GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
	{"mdAudioAlarmStatus",     &(runAlarmCfg.alarmInfo[0].audioAlarmStatus), GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL         },
	{"mdAudioFiles",           &(runAlarmCfg.audioFiles),        GK_CFG_DATA_TYPE_STRING,   "md_alarm.alaw", 1, 1, 384, NULL },
    {"mdAudioFilesCur",        &(runAlarmCfg.alarmInfo[0].audioFilesCur),        GK_CFG_DATA_TYPE_STRING,   "md_alarm.alaw", 1, 1, 128, NULL },
	{"mdAudioPlayCnt",         &(runAlarmCfg.alarmInfo[0].audioPlayCnt),      GK_CFG_DATA_TYPE_S8,   "1", 1, 1, 6, NULL  },
    {"mdLightAlarmStatus",     &(runAlarmCfg.alarmInfo[0].lightAlarmStatus),  GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL },
    {"mdLightAlarmType",       &(runAlarmCfg.alarmInfo[0].lightAlarmType),   GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
    {"mdLightAlarmFrequency",  &(runAlarmCfg.alarmInfo[0].lightAlarmFrequency), GK_CFG_DATA_TYPE_S8,   "2", 1, 0, 3, NULL      },
    {"mdLightAlarmTime",       &(runAlarmCfg.alarmInfo[0].lightAlarmTime),   GK_CFG_DATA_TYPE_S8,   "20", 1, 0, 30, NULL},

	{"areaGbStatus",             &(runAlarmCfg.alarmInfo[1].gbStatus),       GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
	{"areaAudioAlarmStatus",     &(runAlarmCfg.alarmInfo[1].audioAlarmStatus), GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL         },
    {"areaAudioFilesCur",        &(runAlarmCfg.alarmInfo[1].audioFilesCur),        GK_CFG_DATA_TYPE_STRING,   "md_alarm.alaw", 1, 1, 32, NULL },
	{"areaAudioPlayCnt",         &(runAlarmCfg.alarmInfo[1].audioPlayCnt),      GK_CFG_DATA_TYPE_S8,   "1", 1, 1, 6, NULL  },
    {"areaLightAlarmStatus",     &(runAlarmCfg.alarmInfo[1].lightAlarmStatus),  GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL },
    {"areaLightAlarmType",       &(runAlarmCfg.alarmInfo[1].lightAlarmType),   GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
    {"areaLightAlarmFrequency",  &(runAlarmCfg.alarmInfo[1].lightAlarmFrequency), GK_CFG_DATA_TYPE_S8,   "2", 1, 0, 3, NULL      },
    {"areaLightAlarmTime",       &(runAlarmCfg.alarmInfo[1].lightAlarmTime),   GK_CFG_DATA_TYPE_S8,   "20", 1, 0, 30, NULL},
    {NULL,},
};

#else
GK_CFG_MAP alarmInMap[] = {
    {"alarmIn.channel",      &(runAlarmCfg.alarmIn.channel),            },
    {"alarmIn.alarmName",    &(runAlarmCfg.alarmIn.alarmInName),        },
    {"alarmIn.defaultState", &(runAlarmCfg.alarmIn.defaultState),       },
    {"alarmIn.activeState",  &(runAlarmCfg.alarmIn.activeState),        },

    {"alarmIn.handle.intervalTime", &(runAlarmCfg.alarmIn.handle.intervalTime), },
    {"alarmIn.handle.is_snap",      &(runAlarmCfg.alarmIn.handle.is_snap), 		},
	{"alarmIn.handle.snapNum",	  	&(runAlarmCfg.alarmIn.handle.snapNum), 		},
    {"alarmIn.handle.is_beep",      &(runAlarmCfg.alarmIn.handle.is_beep), 		},
	{"alarmIn.handle.beepTime",	  	&(runAlarmCfg.alarmIn.handle.beepTime),		},
    {"alarmIn.scheduleTime", 		&(runAlarmCfg.alarmIn.scheduleTime[0][0]),  },
    {NULL,},
};

GK_CFG_MAP alarmOutMap[] = {
    {"alarmOut.channel",       &(runAlarmCfg.alarmOut.channel),                },
    {"alarmOut.alarmOutName",  &(runAlarmCfg.alarmOut.alarmOutName),           },
    {"alarmOut.defaultState",  &(runAlarmCfg.alarmOut.defaultState),           },
    {"alarmOut.activeState",   &(runAlarmCfg.alarmOut.activeState),            },
    {"alarmOut.powerOnState",  &(runAlarmCfg.alarmOut.powerOnState),           },
    {"alarmOut.pulseDuration", &(runAlarmCfg.alarmOut.pulseDuration),          },
    {"alarmOut.scheduleTime",  &(runAlarmCfg.alarmOut.scheduleTime[0][0]),     },
    {NULL,},
};

GK_CFG_MAP alarmCfgMap[] = {
	{"gpio_enable",           &(runAlarmCfg.cfg.gpio_enable),       },
	{"in_level",              &(runAlarmCfg.cfg.in_level),          },
	{"out_level",             &(runAlarmCfg.cfg.out_level),         },
	{"io_linkage",            &(runAlarmCfg.cfg.io_linkage),        },

    {"upload_interval",       &(runAlarmCfg.cfg.upload_interval),   },
    {"alarmbymail",           &(runAlarmCfg.cfg.alarmbymail),       },
    {"schedule_enable",       &(runAlarmCfg.cfg.schedule_enable),   },
    {"snapshot_enable",       &(runAlarmCfg.cfg.snapshot_enable),   },
    {"recorder_enable",       &(runAlarmCfg.cfg.recorder_enable),   },
    {"move_perset",           &(runAlarmCfg.cfg.move_perset),       },
    {"alarmMute",             &(runAlarmCfg.cfg.alarmMute),         },
    {NULL,},
};

GK_CFG_MAP alarmMojingMap[] = {
	{"mdGbStatus",             &(runAlarmCfg.alarmInfo[0].gbStatus),       GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
	{"mdAudioAlarmStatus",     &(runAlarmCfg.alarmInfo[0].audioAlarmStatus), GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL         },
	{"mdAudioFiles",           &(runAlarmCfg.audioFiles),        GK_CFG_DATA_TYPE_STRING,   "md_alarm.alaw", 1, 1, 384, NULL },
    {"mdAudioFilesCur",        &(runAlarmCfg.alarmInfo[0].audioFilesCur),        GK_CFG_DATA_TYPE_STRING,   "md_alarm.alaw", 1, 1, 128, NULL },
	{"mdAudioPlayCnt",         &(runAlarmCfg.alarmInfo[0].audioPlayCnt),      GK_CFG_DATA_TYPE_S8,   "1", 1, 1, 6, NULL  },
    {"mdLightAlarmStatus",     &(runAlarmCfg.alarmInfo[0].lightAlarmStatus),  GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL },
    {"mdLightAlarmType",       &(runAlarmCfg.alarmInfo[0].lightAlarmType),   GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
    {"mdLightAlarmFrequency",  &(runAlarmCfg.alarmInfo[0].lightAlarmFrequency), GK_CFG_DATA_TYPE_S8,   "2", 1, 0, 3, NULL      },
    {"mdLightAlarmTime",       &(runAlarmCfg.alarmInfo[0].lightAlarmTime),   GK_CFG_DATA_TYPE_S8,   "20", 1, 0, 30, NULL},

	{"areaGbStatus",             &(runAlarmCfg.alarmInfo[1].gbStatus),       GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
	{"areaAudioAlarmStatus",     &(runAlarmCfg.alarmInfo[1].audioAlarmStatus), GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL         },
    {"areaAudioFilesCur",        &(runAlarmCfg.alarmInfo[1].audioFilesCur),        GK_CFG_DATA_TYPE_STRING,   "md_alarm.alaw", 1, 1, 32, NULL },
	{"areaAudioPlayCnt",         &(runAlarmCfg.alarmInfo[1].audioPlayCnt),      GK_CFG_DATA_TYPE_S8,   "1", 1, 1, 6, NULL  },
    {"areaLightAlarmStatus",     &(runAlarmCfg.alarmInfo[1].lightAlarmStatus),  GK_CFG_DATA_TYPE_S8,   "2", 1, -1, 2, NULL },
    {"areaLightAlarmType",       &(runAlarmCfg.alarmInfo[1].lightAlarmType),   GK_CFG_DATA_TYPE_S8,   "1", 1, 0, 2, NULL},
    {"areaLightAlarmFrequency",  &(runAlarmCfg.alarmInfo[1].lightAlarmFrequency), GK_CFG_DATA_TYPE_S8,   "2", 1, 0, 3, NULL      },
    {"areaLightAlarmTime",       &(runAlarmCfg.alarmInfo[1].lightAlarmTime),   GK_CFG_DATA_TYPE_S8,   "20", 1, 0, 30, NULL},
    {NULL,},
};

#endif


void AlarmCfgPrint()
{
    printf("*************** Alarm **************\n");

    printf("alarm in:\n");
    CfgPrintMap(alarmInMap);
    printf("\n");

    printf("alarm out:\n");
    CfgPrintMap(alarmOutMap);
    printf("\n");

	printf("alarm cfg:\n");
    CfgPrintMap(alarmCfgMap);
    printf("\n");
    
	printf("alarm mojing cfg:\n");
    CfgPrintMap(alarmMojingMap);
    printf("\n");

    printf("*************** Alarm **************\n\n");
}


int AlarmCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, "alarmin", alarmInMap);
    CfgAddCjson(root, "alarmout", alarmOutMap);
    CfgAddCjson(root, "alarmcfg", alarmCfgMap);
    CfgAddCjson(root, "alarmMojing", alarmMojingMap);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(ALARM_CFG_FILE, out);
    if (ret != 0) {
        PRINT_ERR("CfgWriteToFile %s error.", ALARM_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int AlarmCfgLoadDefValue()
{
    CfgLoadDefValue(alarmInMap);
    CfgLoadDefValue(alarmOutMap);
	CfgLoadDefValue(alarmCfgMap);
	CfgLoadDefValue(alarmMojingMap);

    return 0;
}


int AlarmCfgLoad()
{
    int load_def_config = 0;
    char *data = NULL;

start_load:
    data = NULL;
    data = CfgReadFromFile(ALARM_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        PRINT_INFO("load %s error, so to load default cfg param.\n", ALARM_CFG_FILE);
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

    CfgParseCjson(json, "alarmin", alarmInMap);
    CfgParseCjson(json, "alarmout", alarmOutMap);
	CfgParseCjson(json, "alarmcfg", alarmCfgMap);
	CfgParseCjson(json, "alarmMojing", alarmMojingMap);

    cJSON_Delete(json);
    free(data);
    return 0;

err:
    if(!load_def_config)
    {
        load_def_config = 1;
        char src_name[64], des_name[64];
        memset(src_name, 0, sizeof(src_name));
        snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, ALARM_CFG_FILE);
        memset(des_name, 0, sizeof(des_name));
        snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, ALARM_CFG_FILE);
        if(!copy_file(src_name, des_name))
            goto start_load;
    }
    AlarmCfgLoadDefValue();
    AlarmCfgSave();
    return 0;
}



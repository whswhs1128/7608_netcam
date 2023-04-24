/*!
*****************************************************************************
** FileName     : cfg_gb28181.c
**
** Description  : gb28181 config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "cfg_all.h"
#include "flash_ctl.h"
#include "cfg_gb28181.h"

GK_NET_GB28181_CFG runGB28181Cfg;

#if USE_DEFUALT_PARAM
GK_CFG_MAP gb28181Map[] = {
    {"ServerIP",           	 &runGB28181Cfg.ServerIP,	GK_CFG_DATA_TYPE_STRING, "gateway.mj.sctel.com.cn", 1, 1, 50, NULL},
    {"ServerPort",           &runGB28181Cfg.ServerPort,	GK_CFG_DATA_TYPE_S32,     "5060",                1, 0, 65535, NULL},
    {"ServerUID",            &runGB28181Cfg.ServerUID,	GK_CFG_DATA_TYPE_STRING, "51000000002006000002", 1, 1, 50, NULL},
    {"ServerPwd",     		 &runGB28181Cfg.ServerPwd,	GK_CFG_DATA_TYPE_STRING, "2KE496r8a", 1, 1, 50, NULL},
    {"ServerDomain", 		 &runGB28181Cfg.ServerDomain, GK_CFG_DATA_TYPE_STRING, "34020000", 1, 1, 50, NULL},
    {"DevicePort", 		 	 &runGB28181Cfg.DevicePort,    GK_CFG_DATA_TYPE_S32, "5060", 1, 1, 65535, NULL},
    {"DeviceUID",  			 &runGB28181Cfg.DeviceUID,   GK_CFG_DATA_TYPE_STRING, "0000000000000000000", 1, 1, 50, NULL},
    {"Expire",     			 &runGB28181Cfg.Expire,   GK_CFG_DATA_TYPE_S32, "3600", 1, 1, 65535, NULL},
	{"DevHBCycle",	  		 &runGB28181Cfg.DevHBCycle,    GK_CFG_DATA_TYPE_S32, "30", 1, 1, 65535, NULL},
	{"DevHBOutTimes",	  	 &runGB28181Cfg.DevHBOutTimes,   GK_CFG_DATA_TYPE_S32, "3", 1, 1, 65535, NULL},
	{"GBWarnEnable",	  	 &runGB28181Cfg.GBWarnEnable,   GK_CFG_DATA_TYPE_U8, "1", 1, 0, 1, NULL},
    {"AlarmID",    			 &runGB28181Cfg.AlarmID,   GK_CFG_DATA_TYPE_STRING, "0000000000000000000", 1, 1, 50, NULL},
    {NULL,},
};

#else
GK_CFG_MAP gb28181Map[] = {
    {"ServerIP",           	 &runGB28181Cfg.ServerIP,	},
    {"ServerPort",           &runGB28181Cfg.ServerPort,	},
    {"ServerUID",            &runGB28181Cfg.ServerUID,	},
    {"ServerPwd",     		 &runGB28181Cfg.ServerPwd,	},
    {"ServerDomain", 		 &runGB28181Cfg.ServerDomain, },
    {"DevicePort", 		 	 &runGB28181Cfg.DevicePort,    },
    {"DeviceUID",  			 &runGB28181Cfg.DeviceUID,   },
    {"Expire",     			 &runGB28181Cfg.Expire,    },
	{"DevHBCycle",	  		 &runGB28181Cfg.DevHBCycle,    },
	{"DevHBOutTimes",	  	 &runGB28181Cfg.DevHBOutTimes,    },
	{"GBWarnEnable",	  	 &runGB28181Cfg.GBWarnEnable,   GK_CFG_DATA_TYPE_U8, "1", 1, 0, 1, NULL},
    {"AlarmID",    			 &runGB28181Cfg.AlarmID,    },
    {NULL,},
};

#endif

void GB28181CfgPrint()
{
    printf("********** GB28181 *********\n");
    CfgPrintMap(gb28181Map);
    printf("********** GB28181 *********\n\n");
}

int GB28181CfgSave(void)
{
    int ret = CfgSave(GB28181_CFG_FILE, "gb28181", gb28181Map);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", GB28181_CFG_FILE);
        return -1;
    }

    return 0;
}

int gb28181_load_id_check(char *id)
{
    int i; 
    int len = strlen(id);
    for (i = 0; i < len; i++)
    {
        if ((id[i] >= '0' && id[i] <= '9') || (id[i] >= 'A' && id[i] <= 'F') || (id[i] >= 'a' && id[i] <= 'f'))
        {
            continue;
        }
        else
        {
            return 1;
        }
    }
    return 0;
}


void gb28181_load_id(void)
{
    char tmpId[32] = {0};
    struct device_info_mtd yunni_info;
    memset(&yunni_info,0,sizeof(struct device_info_mtd));
    int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &yunni_info, sizeof(struct device_info_mtd));
    if((ret < 0) || (yunni_info.device_id[0] == 0))
    {
        printf("Fail to load gb28181 id from flash!\n");
    }
    else
    {
        printf("load gb28181 id:%s\n", yunni_info.device_id);

        //避免flash数据错误导致dhcp失败
        if (gb28181_load_id_check(yunni_info.device_id))
        {
            strcpy(tmpId, "0000000000000000000");
        }
        else
        {
            strcpy(tmpId, yunni_info.device_id);
        }
        if (strcmp(runGB28181Cfg.DeviceUID, tmpId) != 0)
        {
            if (strcmp(tmpId, "000000000") == 0 || strcmp(tmpId, "0000000000000000000") == 0)
            {
                printf("recover id from %s to %s\n", tmpId, runGB28181Cfg.DeviceUID);
                //保存的id为0，但是魔镜id不为0，说明flash存储的id丢失，以魔镜id为准
                strcpy(yunni_info.device_id, runGB28181Cfg.DeviceUID);                
                save_info_to_mtd_reserve(MTD_TUTK_P2P, &yunni_info, sizeof(yunni_info));
            }
            else
            {
                strcpy(runGB28181Cfg.DeviceUID, tmpId);
                GB28181CfgSave();
            }
        }
    }
}


int GB28181CfgLoad(void)
{
    int ret = CfgLoad(GB28181_CFG_FILE, "gb28181", gb28181Map);
    if (ret != 0) {
        PRINT_ERR("CfgLoad %s error.", GB28181_CFG_FILE);
        return -1;
    }

    gb28181_load_id();

    if (strstr(runGB28181Cfg.DeviceUID, "08108") != NULL)
    {
        //runGB28181Cfg.DeviceUID[4] = '2';
    }

	if (strncmp(runSystemCfg.deviceInfo.serialNumber, runGB28181Cfg.DeviceUID, MAX_SYSTEM_STR_SIZE) != 0)
	{
        strcpy(runSystemCfg.deviceInfo.serialNumber, runGB28181Cfg.DeviceUID);
        SystemCfgSave();
	}

    return 0;
}

int GB28181CfgLoadDefValue(void)
{
    CfgLoadDefValue(gb28181Map);

    return 0;
}

cJSON *GB28181CfgGetJsonSting()
{
    cJSON *json = CfgDataToCjsonByMap(gb28181Map);
    return json;
}

int GB28181CfgCheckSave(GK_NET_GB28181_CFG gb28181Cfg)
{
    int ret = 0;
    if (strcmp("*******", gb28181Cfg.ServerPwd) == 0)
    {
        //not change
        strcpy(gb28181Cfg.ServerPwd, runGB28181Cfg.ServerPwd);
        //printf("pwd not change\n");
    }
    if (strcmp("*******", gb28181Cfg.ServerIP) == 0)
    {
        //not change
        strcpy(gb28181Cfg.ServerIP, runGB28181Cfg.ServerIP);
        //printf("ServerIP not change\n");
    }
    if (memcmp(&gb28181Cfg, &runGB28181Cfg, sizeof(GK_NET_GB28181_CFG) - 52) != 0)//not compare alarm id
    {
        memcpy(&runGB28181Cfg, &gb28181Cfg, sizeof(GK_NET_GB28181_CFG) - 52);
        ret = GB28181CfgSave();
        
        struct device_info_mtd factory;
        memset(&factory, 0, sizeof(struct device_info_mtd));
        ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &factory, sizeof(struct device_info_mtd));
        if(ret == 0)
        {
            strcpy(factory.device_id, runGB28181Cfg.DeviceUID);
			save_info_to_mtd_reserve(MTD_TUTK_P2P, &factory, sizeof(struct device_info_mtd));
        }
    }
    else
    {
        PRINT_INFO("gb28181Cfg not change\n");
    }
	return ret;
}

int GB28181Cfg_get(void *dest)
{
	memcpy(dest, &runGB28181Cfg, sizeof(GK_NET_GB28181_CFG));
	return 0;
}
int GB28181Cfg_set(void *src)
{
	memcpy(&runGB28181Cfg, src, sizeof(GK_NET_GB28181_CFG));
	return 0;
}

void  GB28181Cfg_init(void)
{
	PRIVETE_CFG_T GB28181Cfg;

	GB28181Cfg.CfgId = GB28181_PARAM_ID;
	GB28181Cfg.PriveteCfgAddr = &runGB28181Cfg;
	GB28181Cfg.PriveteCfgSize = sizeof(GK_NET_GB28181_CFG);
	GB28181Cfg.privete_cfg_get_param = GB28181Cfg_get;
	GB28181Cfg.privete_cfg_set_param = GB28181Cfg_set;
	GB28181Cfg.privete_cfg_Load = GB28181CfgLoad;
	GB28181Cfg.privete_cfg_Load_def_value = GB28181CfgLoadDefValue;
	GB28181Cfg.privete_cfg_save = GB28181CfgSave;
	reg_privete_cfg_api(&GB28181Cfg);
}




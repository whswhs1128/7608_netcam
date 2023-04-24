/*!
*****************************************************************************
** FileName     : cfg_guyee.c
**
** Description  : guyee config api.
**
** Author       : qiuqiwei <qiuqiwei@goke.com>
** Create Date  : 2018-01-18
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_pwm.h"

extern int copy_file(char *src_name, char *des_name);

#define PWM_CFG_FILE "gk_pwm_iso_cfg.cjson"

GK_AUTO_LIGHT_CFG runpwmCfg;

GK_CFG_MAP pwmMap[] = {
	{"pwm_mode",   &runpwmCfg.pwm_mode,   },
	{"pwm_duty",   &runpwmCfg.pwm_duty,   },
	{"level",      &runpwmCfg.level,      },
    {"pwm_Freq",   &runpwmCfg.pwm_Freq,   },
    {NULL,},
};

GK_CFG_MAP pwmAlMap[ISO_AL_MAX_NUMBER][ISO_AL_MAX_ITEM_NUM] ={
	{
		{"level",     &runpwmCfg.autolight[0].level,    },
		{"pwm_duty",  &runpwmCfg.autolight[0].pwm_duty, },
	    {"gain",      &runpwmCfg.autolight[0].gain,     },
	    {"Dvalue",    &runpwmCfg.autolight[0].Dvalue,   },
	    {NULL,},
	},
	{
		{"level",     &runpwmCfg.autolight[1].level,    },
		{"pwm_duty",  &runpwmCfg.autolight[1].pwm_duty, },
	    {"gain",      &runpwmCfg.autolight[1].gain,     },
	    {"Dvalue",    &runpwmCfg.autolight[1].Dvalue,   },
	    {NULL,},
	},
	{
		{"level",     &runpwmCfg.autolight[2].level,    },
		{"pwm_duty",  &runpwmCfg.autolight[2].pwm_duty, },
	    {"gain",      &runpwmCfg.autolight[2].gain,     },
	    {"Dvalue",    &runpwmCfg.autolight[2].Dvalue,   },
	    {NULL,},
	},
	{
		{"level",     &runpwmCfg.autolight[3].level,    },
		{"pwm_duty",  &runpwmCfg.autolight[3].pwm_duty, },
	    {"gain",      &runpwmCfg.autolight[3].gain,     },
	    {"Dvalue",    &runpwmCfg.autolight[3].Dvalue,   },
	    {NULL,},
	},
	{
		{"level",     &runpwmCfg.autolight[4].level,    },
		{"pwm_duty",  &runpwmCfg.autolight[4].pwm_duty, },
	    {"gain",      &runpwmCfg.autolight[4].gain,     },
	    {"Dvalue",    &runpwmCfg.autolight[4].Dvalue,   },
	    {NULL,},
	},
};

void pwmCfgPrint()
{
	int i;
    printf("********** pwmMap *********\n");
    CfgPrintMap(pwmMap);
	
    printf("********** pwmAlMap *********\n");
    for(i = 0; i < ISO_AL_MAX_NUMBER; i ++)
	{
        printf("pwmAlMap-%d:\n", i);
        CfgPrintMap(pwmAlMap[i]);
        printf("\n");
    }
    printf("********** pwmAlMap *********\n\n");
	
    printf("********** pwmMap *********\n\n");
}

int pwmCfgSave()
{
	int i;
    cJSON *root, *array, *item;
    char *out;

    array = cJSON_CreateArray();
	
    for (i = 0; i < ISO_AL_MAX_NUMBER; i ++)
	{
        item = CfgDataToCjsonByMap(pwmAlMap[i]);
        cJSON_AddItemToArray(array, item);
    }
	
    root = cJSON_CreateObject();//创建项目
    CfgAddCjson(root, "pwm", pwmMap);
    cJSON_AddItemToObject(root, "autolight", array);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(PWM_CFG_FILE, out);
    if (ret != 0)
	{
        PRINT_ERR("CfgWriteToFile %s error.", PWM_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int pwmCfgLoadDefValue()
{
	int i;
    for (i = 0; i < ISO_AL_MAX_NUMBER; i ++)
	{
        CfgLoadDefValue(pwmAlMap[i]);
    }

    CfgLoadDefValue(pwmMap);
    return 0;
}

int pwmCfgLoad()
{
	int i;
	int load_def_config = 0;
	char *data = NULL;

start_load:
	data = CfgReadFromFile(PWM_CFG_FILE);
	if (data == NULL)
	{
		//从配置文件读取失败，则使用默认参数
		PRINT_INFO("load %s error, so to load default cfg param.\n", PWM_CFG_FILE);
		goto err;
	}

	cJSON *root = cJSON_Parse(data);
	if (!root)
	{
		//从配置文件解析cjson失败，则使用默认参数
		PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
		free(data);
		goto err;
	}

	CfgParseCjson(root, "pwm", pwmMap);
	
	// get channel osd
    cJSON *rfid = cJSON_GetObjectItem(root, "autolight");
    if(!rfid)
	{
        PRINT_ERR("get autolight cJSON_GetObjectItem error\n");
        goto err1;
    }

	int arraySize = cJSON_GetArraySize(rfid);
	PRINT_INFO("autolight max:%d, arraySize:%d\n", ISO_AL_MAX_NUMBER, arraySize);

    cJSON *arrayItem = NULL;
    for(i = 0; i < arraySize; i++)
	{
        arrayItem = cJSON_GetArrayItem(rfid, i);
        if(!arrayItem)
		{
            PRINT_ERR("cJSON_GetArrayItem error\n");
            goto err1;
        }
        CfgCjsonToDataByMap(pwmAlMap[i], arrayItem);
    }
		
	cJSON_Delete(root);
	free(data);
	pwmCfgSave();
	//pwmCfgPrint();
	return 0;
	err1:
		cJSON_Delete(root);
		free(data);

err:
	if(!load_def_config)
	{
		load_def_config = 1;
		char src_name[64], des_name[64];
		memset(src_name, 0, sizeof(src_name));
		snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, PWM_CFG_FILE);
		memset(des_name, 0, sizeof(des_name));
		snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, PWM_CFG_FILE);
		if(!copy_file(src_name, des_name))
			goto start_load;
	}
	pwmCfgLoadDefValue();
	pwmCfgSave();
	return 0;
}

char *pwmCfgGetJosnString(void)
{
    char *out;
    cJSON *root = CfgDataToCjsonByMap(pwmMap);
    if(root == NULL)
    {
        PRINT_ERR("pwmMap to json error.\n");
        return NULL;
    }
    out = cJSON_Print(root);
    //PRINT_INFO("pwm json:%s\n",out);
    cJSON_Delete(root);
    return out;

}


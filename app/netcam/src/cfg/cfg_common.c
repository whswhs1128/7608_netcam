/*!
*****************************************************************************
** FileName     : cfg_common.c
**
** Description  : common api of config module.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-31
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_common.h"
#include "cfg_all.h"

pthread_mutex_t g_cfg_write_mutex = PTHREAD_MUTEX_INITIALIZER;

extern int copy_file(char *src_name, char *des_name);

static int cfg_get_data_type(char *type)
{
    char etype[10][8]={"U32","U16","U8","S32","S16","S8","FLOAT","STRING","STIME","SLICE"};
    int i = 0;
    while(i < 10)
    {
        if(strcmp(type,etype[i]) == 0)
        {
            break;
        }
        i++;
    }
    return i;
}

int CfgWriteToFile(const char *filename, const char *data)
{
    pthread_mutex_lock(&g_cfg_write_mutex);
    char path[100];

    memset(path, 0, sizeof(path));
    sprintf(path, "%s%s", CFG_DIR, filename);
    PRINT_INFO("to save %s\n", path);

    FILE *fp = NULL;
    fp = fopen(path, "wb+");
    if (fp == NULL) {
        PRINT_ERR("fopen:%s fail\n", path);
        pthread_mutex_unlock(&g_cfg_write_mutex);
        return -1;
    }

    int len = strlen(data);
    if (fwrite(data, 1, len, fp) != len) {
        PRINT_ERR("fwrite:%s fail\n", path);
        fclose(fp);
        pthread_mutex_unlock(&g_cfg_write_mutex);
        return -1;
    }

	fflush(fp);
    fclose(fp);
    pthread_mutex_unlock(&g_cfg_write_mutex);
    return 0;
}

char *CfgReadFromFile(const char *filename)
{
    char path[100];

    //目录不存在，则创建
    if(access(CFG_DIR, F_OK) != 0 ) {
        PRINT_INFO("to create cfg dir: %s\n", CFG_DIR);
        if((mkdir(CFG_DIR, 0777)) < 0)
        {
            PRINT_ERR("mkdir %s failed\n", CFG_DIR);
            return NULL;
        }
    }

    memset(path, 0, sizeof(path));
    sprintf(path, "%s%s", CFG_DIR, filename);
    //PRINT_INFO("to load %s\n", path);

    FILE *fp = NULL;
    fp = fopen(path, "rb");
    if (fp == NULL) {

        return NULL;
    }

    int fileSize;
    if (0 != fseek(fp, 0, SEEK_END)) {
        fclose(fp);
        return NULL;
    }
    fileSize = ftell(fp);

    char *data = NULL;
    data = malloc(fileSize);
    if(!data) {
        PRINT_ERR();
        fclose(fp);
        return NULL;
    }
    memset(data, 0, fileSize);

    if(0 != fseek(fp, 0, SEEK_SET)) {
        PRINT_ERR();
        fclose(fp);
        return NULL;
    }
    if (fread(data, 1, fileSize, fp) != (fileSize)) {
        PRINT_ERR();
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    return data;
}


char *CfgReadJsonFile(const char *filename)
{
    //PRINT_INFO("to load %s\n", filename);
    FILE *fp = NULL;
    fp = fopen(filename, "rb");
    if (fp == NULL) {

        return NULL;
    }

    int fileSize;
    if (0 != fseek(fp, 0, SEEK_END)) {
        fclose(fp);
        return NULL;
    }
    fileSize = ftell(fp);

    char *data = NULL;
    data = malloc(fileSize);
    if(!data) {
        PRINT_ERR();
        fclose(fp);
        return NULL;
    }
    memset(data, 0, fileSize);

    if(0 != fseek(fp, 0, SEEK_SET)) {
        PRINT_ERR();
        fclose(fp);
        return NULL;
    }
    if (fread(data, 1, fileSize, fp) != (fileSize)) {
        PRINT_ERR();
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    return data;
}

static int CfgLoadDefValueItem(GK_CFG_MAP *map)
{
    int tmp;
	double temp;
    switch (map->dataType) {
        case GK_CFG_DATA_TYPE_U32:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned int *)(map->dataAddress)) = (unsigned int)tmp;
            break;
        case GK_CFG_DATA_TYPE_U16:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned short *)(map->dataAddress)) = (unsigned short)tmp;
            break;
        case GK_CFG_DATA_TYPE_U8:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned char *)(map->dataAddress)) = (unsigned char)tmp;
            break;
        case GK_CFG_DATA_TYPE_S32:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((int *)(map->dataAddress)) = tmp;
            break;
        case GK_CFG_DATA_TYPE_S16:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed short *)(map->dataAddress)) = (signed short)tmp;
            break;
        case GK_CFG_DATA_TYPE_S8:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed char *)(map->dataAddress)) = (signed char)tmp;
            break;
		case GK_CFG_DATA_TYPE_FLOAT:
			temp = strtod(map->defaultValue, NULL);
            *(( float *)(map->dataAddress)) = (float)temp;
            break;

        case GK_CFG_DATA_TYPE_STRING:
			if (map->dataAddress && (map->max > 1)) {
				tmp = (int)map->max - 1;
                strncpy((char *)map->dataAddress, map->defaultValue, tmp);
                ((char *)map->dataAddress)[tmp] = '\0';
            } else {
                PRINT_ERR("if type is string, addr can't be null and the upper limit must greater than 1.");
            }
            break;
        case GK_CFG_DATA_TYPE_STIME:
            {
                GK_SCHEDTIME *stime = (GK_SCHEDTIME *)map->dataAddress;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 4; j ++) {
                        stime->startHour = 0;
                        stime->startMin = 0;
                        stime->stopHour = 23;
                        stime->stopMin = 59;
                        stime ++;
                    }
                }
            }
            break;
        case GK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)map->dataAddress;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        *slice = strtoul("4294967295", NULL, 0);
                        slice ++;
                    }
                }
            }
            break;
        default:
            PRINT_ERR("unknown data type in map definition!\n");
            break;
    }

    return 0;
}

int CfgLoadDefValue(GK_CFG_MAP *mapArray)
{
    GK_CFG_MAP *map = NULL;
    int i =0;
    while (mapArray[i].stringName != NULL) {
        map = &mapArray[i];
        CfgLoadDefValueItem(map);
        i++;
    }

    return 0;
}

static int Cfg_Add_Property(GK_CFG_MAP *map, cJSON *root)
{
	cJSON *Item = cJSON_CreateProperty();
	char etype[][8]={"U32","U16","U8","S32","S16","S8","FLOAT","STRING","STIME","SLICE"};
	char strname[128]={0};
	sprintf(strname,"%s%s",map->stringName,"Property");

	cJSON_AddItemToObject(Item, "type", cJSON_CreateString(etype[map->dataType]));
	cJSON_AddItemToObject(Item, "mode", cJSON_CreateString(map->mode==0?"ro":"rw"));
	cJSON_AddItemToObject(Item, "min", cJSON_CreateNumber(map->min));
	cJSON_AddItemToObject(Item, "max", cJSON_CreateNumber(map->max));
	if(map->dataType < GK_CFG_DATA_TYPE_STRING)
	{
		cJSON_AddItemToObject(Item, "def", cJSON_CreateNumber(strtoul(map->defaultValue, NULL, 0)));
	}
	else
		cJSON_AddItemToObject(Item, "def", cJSON_CreateString(map->defaultValue));
	if(map->description!=NULL)
		cJSON_AddItemToObject(Item, "opt", cJSON_CreateString(map->description));
	else
		cJSON_AddItemToObject(Item, "opt", cJSON_CreateString(""));

    cJSON_AddItemToObject(root, strname, Item);
	//printf("\n%s\n",cJSON_Print(Item));
    return 0;
}


static int CfgDataToCjsonByMapItem(GK_CFG_MAP *map, cJSON *root)
{
    int tmp;
	double temp;

    switch (map->dataType) {
        case GK_CFG_DATA_TYPE_U32:
            tmp = *((unsigned int *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case GK_CFG_DATA_TYPE_U16:
            tmp = *((unsigned short *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case GK_CFG_DATA_TYPE_U8:
            tmp = *((unsigned char *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case GK_CFG_DATA_TYPE_S32:
            tmp = *((signed int *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case GK_CFG_DATA_TYPE_S16:
            tmp = *((signed short *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
        case GK_CFG_DATA_TYPE_S8:
            tmp = *((signed char *)(map->dataAddress));
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            cJSON_AddNumberToObject(root, map->stringName, tmp);
            break;
		case GK_CFG_DATA_TYPE_FLOAT:
            temp = *(( float *)(map->dataAddress));
            if (temp > map->max || temp < map->min)
                temp = strtod(map->defaultValue, NULL);
            cJSON_AddNumberToObject(root, map->stringName, temp);
            break;

        case GK_CFG_DATA_TYPE_STRING:
            cJSON_AddStringToObject(root, map->stringName, (char *)map->dataAddress);//
            break;

        case GK_CFG_DATA_TYPE_STIME:
            {
                GK_SCHEDTIME *stime = (GK_SCHEDTIME *)map->dataAddress;

                int i, j;
                char tmp[20] = {0};
                cJSON *array = cJSON_CreateArray();
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 4; j ++) {
                        sprintf(tmp, "%d:%d-%d:%d", stime->startHour, \
                                                    stime->startMin, \
                                                    stime->stopHour, \
                                                    stime->stopMin);
                        cJSON_AddItemToArray(array, cJSON_CreateString(tmp));
                        stime ++;
                    }
                }
                cJSON_AddItemToObject(root, map->stringName, array);
            }
            break;

        case GK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)map->dataAddress;

                int i, j;
                char tmp[20] = {0};
                cJSON *array = cJSON_CreateArray();
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        sprintf(tmp, "%u", *slice);
                        cJSON_AddItemToArray(array, cJSON_CreateString(tmp));
                        slice ++;
                    }
                }
                cJSON_AddItemToObject(root, map->stringName, array);
            }
            break;
        default:
            PRINT_ERR("unknown data type in map definition!\n");
            break;
    }
    Cfg_Add_Property(map, root);

    return 0;
}



cJSON *CfgDataToCjsonByMap(GK_CFG_MAP *mapArray)
{
    cJSON *root;

    root = cJSON_CreateObject();//创建项目
    int i =0;

    while(mapArray[i].stringName != NULL) {
        CfgDataToCjsonByMapItem(&mapArray[i], root);
        i ++;
    }

    return root;
}

static int CfgCjsonToDataByMapItem(GK_CFG_MAP *map, cJSON *json)
{
    int tmp;
	double temp;

    switch (map->dataType) {
        case GK_CFG_DATA_TYPE_U32:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned int *)(map->dataAddress)) = (unsigned int)tmp;
            break;
        case GK_CFG_DATA_TYPE_U16:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned short *)(map->dataAddress)) = (unsigned short)tmp;
            break;
        case GK_CFG_DATA_TYPE_U8:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned char *)(map->dataAddress)) = (unsigned char)tmp;
            break;
        case GK_CFG_DATA_TYPE_S32:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            *((int *)(map->dataAddress)) = tmp;
            break;
        case GK_CFG_DATA_TYPE_S16:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            *((signed short *)(map->dataAddress)) = (signed short)tmp;
            break;
        case GK_CFG_DATA_TYPE_S8:
            tmp = json->valueint;
            if (tmp > map->max || tmp < map->min)
                tmp = strtol(map->defaultValue, NULL, 0);
            *((signed char *)(map->dataAddress)) = (signed char)tmp;
            break;
		case GK_CFG_DATA_TYPE_FLOAT:
            temp = json->valuedouble;
            if (temp > map->max || temp < map->min)
                temp = strtod(map->defaultValue, NULL);
            *(( float *)(map->dataAddress)) = ( float)temp;
            break;

        case GK_CFG_DATA_TYPE_STRING:
			if (map->dataAddress && (map->max > 1)) {
				tmp = (int)map->max - 1;
                strncpy((char *)map->dataAddress, json->valuestring, tmp);
                ((char *)map->dataAddress)[tmp] = '\0';
            } else {
                PRINT_ERR("if type is string, addr can't be null and the upper limit must greater than 1.");
            }
            break;

        case GK_CFG_DATA_TYPE_STIME:
            {
                GK_SCHEDTIME *stime = (GK_SCHEDTIME *)map->dataAddress;
                cJSON *tmp = NULL;
                int i, j;
                int startHour, startMin, stopHour, stopMin;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 4; j ++) {
                        tmp = cJSON_GetArrayItem(json, i * 4 + j);
                        if (tmp) {
                            //printf("[%d][%d] %s\n", i, j, tmp->valuestring);
                            sscanf(tmp->valuestring, "%d:%d-%d:%d", &startHour, &startMin, &stopHour, &stopMin);
                            stime->startHour = startHour;
                            stime->startMin = startMin;
                            stime->stopHour = stopHour;
                            stime->stopMin = stopMin;
                            stime ++;
                        }

                    }
                }
            }
            break;

        case GK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)map->dataAddress;
                cJSON *tmp = NULL;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        tmp = cJSON_GetArrayItem(json, i * 3 + j);
                        if (tmp) {
                            *slice = strtoul(tmp->valuestring, NULL, 0);
                            slice ++;
                        }
                    }
                }
            }
            break;
        default:
            PRINT_ERR("unknown data type in map definition!\n");
            break;
    }

    return 0;
}


static int Cfg_Property_ByMap(GK_CFG_MAP *map, cJSON *json)
{
	char strname[1024]={0};


	sprintf(strname,"%s%s",map->stringName,"Property");
	cJSON *Item = cJSON_GetObjectItem(json,strname);
	if(Item!=NULL)
	{
		memset(strname, 0, sizeof(strname));
        map->dataType = cfg_get_data_type(cJSON_GetObjectItem(Item,"type")->valuestring);

		if(map->dataType <= GK_CFG_DATA_TYPE_S8)
		{
			sprintf(strname,"%d",cJSON_GetObjectItem(Item,"def")->valueint);
		}
        else if(map->dataType == GK_CFG_DATA_TYPE_FLOAT)
		{
			sprintf(strname,"%f",cJSON_GetObjectItem(Item,"def")->valuedouble);
		}
		else
		{
			strcpy(strname, cJSON_GetObjectItem(Item,"def")->valuestring);
		}
        if(strcmp(cJSON_GetObjectItem(Item,"mode")->valuestring,"ro") == 0)
        {
            map->mode = 0;
        }
        else
        {
            map->mode = 1;
        }
		int len = strlen(strname) + 1;
		len+=(4-len%4);
		map->defaultValue = malloc(len);
		if(map->defaultValue==NULL)
			printf("========= malloc ====\n");
		memset(map->defaultValue, 0, len);

		strncpy(map->defaultValue , strname, len);
		map->min = cJSON_GetObjectItem(Item,"min")->valuedouble;
		map->max = cJSON_GetObjectItem(Item,"max")->valuedouble;

		memset(strname, 0, sizeof(strname));
		strncpy(strname, cJSON_GetObjectItem(Item,"opt")->valuestring,sizeof(strname));
		len = strlen(strname) + 1;
		len+=(4-len%4);
		map->description= malloc(len);
		if(map->description==NULL)
			printf("========= malloc ====\n");
		memset(map->description, 0, len);

		strncpy(map->description , strname, len);
	}
    return 0;
}


int CfgCjsonToDataByMap(GK_CFG_MAP *mapArray, cJSON *json)
{
    int i =0;
    cJSON *tmp = NULL;

    while (mapArray[i].stringName != NULL) {
        tmp = cJSON_GetObjectItem(json, mapArray[i].stringName);
        if (tmp) {
				Cfg_Property_ByMap(&mapArray[i], json);
                CfgCjsonToDataByMapItem(&mapArray[i], tmp);
        } else {
            //从配置文件解析cjson失败，则使用默认参数
            PRINT_INFO("[%s] cjson get failed. use default parameter\n", mapArray[i].stringName);
            CfgLoadDefValueItem(&mapArray[i]);
        }
        i++;
    }

    return 0;
}

static void CfgPrintItem(GK_CFG_MAP *map)
{
    switch (map->dataType) {
        case GK_CFG_DATA_TYPE_U32:
            printf("%s : %u 0x%x\n", map->stringName, *((unsigned int *)(map->dataAddress)), (unsigned int)map->dataAddress);
            break;
        case GK_CFG_DATA_TYPE_U16:
            printf("%s : %u 0x%x\n", map->stringName, *((unsigned short *)(map->dataAddress)), (unsigned int)map->dataAddress);
            break;
        case GK_CFG_DATA_TYPE_U8:
            printf("%s : %u 0x%x\n", map->stringName, *((unsigned char *)(map->dataAddress)), (unsigned int)map->dataAddress);
            break;
        case GK_CFG_DATA_TYPE_S32:
            printf("%s : %d 0x%x\n", map->stringName, *((signed int *)(map->dataAddress)), (unsigned int)map->dataAddress);
            break;
        case GK_CFG_DATA_TYPE_S16:
            printf("%s : %d 0x%x\n", map->stringName, *((signed short *)(map->dataAddress)), (unsigned int)map->dataAddress);
            break;
        case GK_CFG_DATA_TYPE_S8:
            printf("%s : %d 0x%x\n", map->stringName, *((signed char *)(map->dataAddress)), (unsigned int)map->dataAddress);
            break;
		case GK_CFG_DATA_TYPE_FLOAT:
			printf("%s : %f 0x%x\n", map->stringName, *(( float *)(map->dataAddress)), (unsigned int)map->dataAddress);
            break;

        case GK_CFG_DATA_TYPE_STRING:
            printf("%s : %s 0x%x\n", map->stringName, (char *)(map->dataAddress), (unsigned int)map->dataAddress);
            break;

        case GK_CFG_DATA_TYPE_STIME:
            {
                printf("%s : [\n", map->stringName);
                GK_SCHEDTIME *stime = (GK_SCHEDTIME *)map->dataAddress;

                int i, j;
                for (i = 0; i < 7; i ++) {
                    printf("    date %d: ", i);
                    for (j = 0; j < 4; j ++) {
                        printf("%d:%d - %d:%d, ", stime->startHour, \
                                                  stime->startMin, \
                                                  stime->stopHour, \
                                                  stime->stopMin);
                        stime ++;
                    }
                    printf("\n");
                }
                printf("]\n");
            }
            break;

        case GK_CFG_DATA_TYPE_SLICE:
            {
                printf("%s : [\n", map->stringName);
                unsigned int *slice = (unsigned int *)map->dataAddress;

                int i, j;
                for (i = 0; i < 7; i ++) {
                    printf("    date %d: ", i);
                    for (j = 0; j < 3; j ++) {
                        printf("%u, ", (*slice));
                        slice ++;
                    }
                    printf("\n");
                }
                printf("]\n");
            }
            break;
        default:
            PRINT_ERR("unknown data type in map definition!\n");
            break;
    }
}

int CfgPrintMap(GK_CFG_MAP *mapArray)
{
    int i =0;
    GK_CFG_MAP *map = NULL;
    while (mapArray[i].stringName != NULL) {
        map = &mapArray[i];
        CfgPrintItem(map);
        i++;
    }

    return 0;
}

int CfgParseCjson(cJSON *root, const char *str, GK_CFG_MAP *mapArray)
{
    cJSON *item = NULL;
    item = cJSON_GetObjectItem(root, str);
    if (!item) {
        //PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
        return -1;
    }
    CfgCjsonToDataByMap(mapArray, item);
    return 0;
}

int CfgLoad(const char *filename, const char *str, GK_CFG_MAP *mapArray)
{
    int load_def_config = 0;
    char *data = NULL;
    //int isDefault = 0;

start_load:
    data = CfgReadFromFile(filename);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        PRINT_INFO("load %s error, so to load default cfg param.\n", filename);
        goto err2;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json) {
        //从配置文件解析cjson失败，则使用默认参数
        PRINT_INFO("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err2;
    }

    CfgParseCjson(json, str, mapArray);
    #if 0
    if (isDefault && strcmp(filename, "gk_audio_cfg.cjson") == 0)
    {
        PRINT_INFO("load %s default, mute\n", filename);
        runAudioCfg.rebootMute = 1;
    }
    #endif
    cJSON_Delete(json);
    free(data);
    return 0;

err2:
    if(!load_def_config)
    {
        load_def_config = 1;
        //isDefault = 1;
        char src_name[64], des_name[64];
        memset(src_name, 0, sizeof(src_name));
        snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, filename);
        memset(des_name, 0, sizeof(des_name));
        snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, filename);
        if(!copy_file(src_name, des_name))
            goto start_load;
    }
    CfgLoadDefValue(mapArray);
    CfgSave(filename,str, mapArray);
    return 0;
}


int CfgAddCjson(cJSON *root, const char *str, GK_CFG_MAP *mapArray)
{
    cJSON *item = NULL;
    item = CfgDataToCjsonByMap(mapArray);
    cJSON_AddItemToObject(root, str, item);

    return 0;
}

int CfgSave(const char *filename, const char *str, GK_CFG_MAP *mapArray)
{
    int ret = 0;
    char *out;
    cJSON *root;

    root = cJSON_CreateObject();//创建项目

    CfgAddCjson(root, str, mapArray);

    out = cJSON_Print(root);
    ret = CfgWriteToFile(filename, out);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", filename);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int CfgGetDefByName(GK_CFG_MAP *mapArray, const char *item_name, void *value)
{
    if ((value == NULL) || (item_name == NULL) || (mapArray == NULL)) {
        PRINT_ERR("param error!\n");
        return -1;
    }

    GK_CFG_MAP *map = NULL;
    int i =0;
    while (mapArray[i].stringName != NULL) {
        map = &mapArray[i];
        if (strcmp(map->stringName, item_name) == 0)
            break;
        i++;
    }
    if (mapArray[i].stringName == NULL)
        return -1;

    int tmp;
	double temp;
    switch (map->dataType) {
        case GK_CFG_DATA_TYPE_U32:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned int *)value) = (unsigned int)tmp;
            break;
        case GK_CFG_DATA_TYPE_U16:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned short *)value) = (unsigned short)tmp;
            break;
        case GK_CFG_DATA_TYPE_U8:
            tmp = strtoul(map->defaultValue, NULL, 0);
            *((unsigned char *)value) = (unsigned char)tmp;
            break;
        case GK_CFG_DATA_TYPE_S32:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((int *)value) = tmp;
            break;
        case GK_CFG_DATA_TYPE_S16:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed short *)value) = (signed short)tmp;
            break;
        case GK_CFG_DATA_TYPE_S8:
            tmp = strtol(map->defaultValue, NULL, 0);
            *((signed char *)value) = (signed char)tmp;
            break;

		case GK_CFG_DATA_TYPE_FLOAT:
            temp = strtod(map->defaultValue, NULL);
            *(( float *)value) = ( float)temp;
            break;

        case GK_CFG_DATA_TYPE_STRING:
			if (map->max > 1) {
				tmp = (int)map->max - 1;
                strncpy((char *)value, map->defaultValue, tmp);
                ((char *)value)[tmp] = '\0';
            } else {
                PRINT_ERR("if type is string, addr can't be null and the upper limit must greater than 1.");
            }
            break;
        case GK_CFG_DATA_TYPE_STIME:
            {
                GK_SCHEDTIME *stime = (GK_SCHEDTIME *)value;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 4; j ++) {
                        stime->startHour = 0;
                        stime->startMin = 0;
                        stime->stopHour = 23;
                        stime->stopMin = 59;
                        stime ++;
                    }
                }
            }
            break;
        case GK_CFG_DATA_TYPE_SLICE:
            {
                unsigned int *slice = (unsigned int *)value;
                int i, j;
                for (i = 0; i < 7; i ++) {
                    for (j = 0; j < 3; j ++) {
                        *slice = strtoul("4294967295", NULL, 0);
                        slice ++;
                    }
                }
            }
            break;
        default:
            PRINT_ERR("unknown data type in map definition!\n");
            break;
    }

    return 0;
}

int is_in_schedule_slice(SDK_U32 *slice)
{
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);

    unsigned int tmp;
    if (pTm->tm_hour < 8) {
        //tmp = runMdCfg.scheduleSlice[pTm->tm_wday][0];
        tmp = *(slice + pTm->tm_wday * 3 + 0);
        pTm->tm_hour -= 0;
    } else if (pTm->tm_hour > 15) {
        //tmp = runMdCfg.scheduleSlice[pTm->tm_wday][2];
        tmp = *(slice + pTm->tm_wday * 3 + 2);
        pTm->tm_hour -= 16;
    } else {
        //tmp = runMdCfg.scheduleSlice[pTm->tm_wday][1];
        tmp = *(slice + pTm->tm_wday * 3 + 1);
        pTm->tm_hour -= 8;
    }
    //PRINT_INFO("slice:%u\n", tmp);

    int mask1 = -1;
    if (pTm->tm_min < 15)
        mask1 = 0;
    if ((pTm->tm_min >= 15) && (pTm->tm_min < 30))
        mask1 = 1;
    if ((pTm->tm_min >= 30) && (pTm->tm_min < 45))
        mask1 = 2;
    if ((pTm->tm_min >= 45) && (pTm->tm_min < 60))
        mask1 = 3;

    int mask2 = pTm->tm_hour * 4 + mask1;
    //PRINT_INFO("mask:%d\n", mask2);

    int ret = (((1 << mask2) & tmp) == 0) ? 0 : 1;
    //PRINT_INFO("ret:%d\n", ret);

    return ret;
}

int timepoint_to_u64(GK_SCHEDTIME *pSchedTime, unsigned long long *start_64, unsigned long long *stop_64)
{
    char str[30];
    memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pSchedTime->startHour, pSchedTime->startMin);
    *start_64 = atoll(str);

    memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pSchedTime->stopHour, pSchedTime->stopMin);
    *stop_64 = atoll(str);

    return 0;
}


int is_in_schedule_timepoint(GK_SCHEDTIME *time_point)
{
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);

    char str[30];
    memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pTm->tm_hour, pTm->tm_min);
    unsigned long long now_64 = atoll(str);

    int i;
    int ret = 0;
    unsigned long long start_64 = 0;
    unsigned long long stop_64 = 0;
    GK_SCHEDTIME *tPoint = NULL;
    //PRINT_INFO("pTm->tm_wday:%d\n", pTm->tm_wday);
    //for (i = 0; i < 4; i ++) {
    for (i = 0; i < 2; i ++) {//just check the 1&2 timepoint, 3&4 is invalid.
        tPoint = (time_point + pTm->tm_wday * 4 + i);
        timepoint_to_u64(tPoint, &start_64, &stop_64);
        //PRINT_INFO("day:%d, index:%d, time:%llu - %llu, now_time:%llu",
            //pTm->tm_wday, i, start_64, stop_64, now_64);
        if (start_64 <= stop_64) {
            if ((now_64 >= start_64) && (now_64 <= stop_64)) {
                ret = 1;
                break;
            }
        } else {
            if ((now_64 >= start_64) || (now_64 <= stop_64))  {
                ret = 1;
                break;
            }
        }
    }

    return ret;
}

int is_in_schedule_timepoint_report(GK_SCHEDTIME *time_point, int* index)
{
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);

    char str[30];
    memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pTm->tm_hour, pTm->tm_min);
    unsigned long long now_64 = atoll(str);

    int i;
    int ret = 0;
    unsigned long long start_64 = 0;
    unsigned long long stop_64 = 0;
    GK_SCHEDTIME *tPoint = NULL;
    //PRINT_INFO("pTm->tm_wday:%d\n", pTm->tm_wday);
    for (i = 0; i < ACTION_TIME_SEGNUM; i ++) {
        //tPoint = (time_point + pTm->tm_wday * 4 + i);
        tPoint = (time_point + i);
        timepoint_to_u64(tPoint, &start_64, &stop_64);
        //PRINT_INFO("day:%d, index:%d, time:%llu - %llu, now_time:%llu",
          //  pTm->tm_wday, i, start_64, stop_64, now_64);
        if (start_64 <= stop_64) {
            if ((now_64 >= start_64) && (now_64 <= stop_64)) {
                *index = i;
                ret = 1;
                break;
            }
        } else {
            if ((now_64 >= start_64) || (now_64 <= stop_64))  {
                *index = i;
                ret = 1;
                break;
            }
        }
    }

    return ret;
}

int is_in_schedule_timepoint_report_with_cnt(GK_SCHEDTIME *time_point, int scheldueCnt, int* index)
{
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);
    char str[30];
    memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pTm->tm_hour, pTm->tm_min);
    unsigned long long now_64 = atoll(str);

    int i;
    int ret = 0;
    unsigned long long start_64 = 0;
    unsigned long long stop_64 = 0;
    GK_SCHEDTIME *tPoint = NULL;
    //PRINT_INFO("pTm->tm_wday:%d\n", pTm->tm_wday);
    for (i = 0; i < scheldueCnt; i ++) {
        //tPoint = (time_point + pTm->tm_wday * 4 + i);
        tPoint = (time_point + i);
        timepoint_to_u64(tPoint, &start_64, &stop_64);
        //PRINT_INFO("day:%d, index:%d, time:%llu - %llu, now_time:%llu",
          //  pTm->tm_wday, i, start_64, stop_64, now_64);
        if (start_64 <= stop_64) {
            if ((now_64 >= start_64) && (now_64 <= stop_64)) {
                *index = i;
                ret = 1;
                break;
            }
        } else {
            if ((now_64 >= start_64) || (now_64 <= stop_64))  {
                *index = i;
                ret = 1;
                break;
            }
        }
    }

    return ret;
}



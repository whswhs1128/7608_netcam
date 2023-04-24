/*!
*****************************************************************************
** FileName     : cfg_user.c
**
** Description  : user config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_user.h"
#include "sdk_sys.h"
#include <unistd.h>

extern int copy_file(char *src_name, char *des_name);

GK_NET_USER_CFG runUserCfg;

#if USE_DEFAULT_PARAM
static GK_CFG_MAP adminMap[] = {
    {"enable",    &(runUserCfg.user[0].enable),    GK_CFG_DATA_TYPE_S32,    "1",     "rw", 0, 1,              NULL},
    {"userName",  &(runUserCfg.user[0].userName),  GK_CFG_DATA_TYPE_STRING, "admin", "rw", 1, MAX_STR_LEN_64, NULL},
    {"password",  &(runUserCfg.user[0].password),  GK_CFG_DATA_TYPE_STRING, "admin", "rw", 1, MAX_STR_LEN_64, NULL},
    {"userRight", &(runUserCfg.user[0].userRight), GK_CFG_DATA_TYPE_U32,    "2", "rw", 0,  2,          NULL},
	{"userLevel", &(runUserCfg.user[0].userLevel), GK_CFG_DATA_TYPE_S32,	"1", 	 "rw", 1,  5,		   	  NULL},
    {NULL,},
};


#else
static GK_CFG_MAP adminMap[] = {
    {"enable",    &(runUserCfg.user[0].enable),     },
    {"userName",  &(runUserCfg.user[0].userName),   },
    {"password",  &(runUserCfg.user[0].password),   },
    {"userRight", &(runUserCfg.user[0].userRight),  },
	{"userLevel", &(runUserCfg.user[0].userLevel),  },
    {NULL,},
};
#endif

void UserCfgPrint()
{
    printf("*************** User **************\n");

    //CfgPrintMap(adminMap);

    int i;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        if (runUserCfg.user[i].enable) {
            printf("user%d -- name: %s, password:%s right:%u level: %d\n", i,
                runUserCfg.user[i].userName,
                runUserCfg.user[i].password,
                runUserCfg.user[i].userRight,
                runUserCfg.user[i].userLevel);
        }
    }

    printf("*************** User **************\n\n");
}


int UserCfgSave()
{
    cJSON *root;
    char *out;

    root = cJSON_CreateObject();//创建项目
    int i = 0;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        if (runUserCfg.user[i].enable == 1) {
            char cjson_name[10] = {0};
            sprintf(cjson_name, "user_%d", i);
            adminMap[0].dataAddress = &(runUserCfg.user[i].enable);
            adminMap[1].dataAddress = &(runUserCfg.user[i].userName);
            adminMap[2].dataAddress = &(runUserCfg.user[i].password);
            adminMap[3].dataAddress = &(runUserCfg.user[i].userRight);
            adminMap[4].dataAddress = &(runUserCfg.user[i].userLevel);
            CfgAddCjson(root, cjson_name, adminMap);
        }
    }

    adminMap[0].dataAddress = &(runUserCfg.user[0].enable);
    adminMap[1].dataAddress = &(runUserCfg.user[0].userName);
    adminMap[2].dataAddress = &(runUserCfg.user[0].password);
    adminMap[3].dataAddress = &(runUserCfg.user[0].userRight);
    adminMap[4].dataAddress = &(runUserCfg.user[0].userLevel);
    out = cJSON_Print(root);

    int ret = CfgWriteToFile(USER_CFG_FILE, out);
    if (ret != 0) {
        PRINT_ERR("CfgWriteToFile %s error.", USER_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int UserCfgLoadDefValue()
{
    int i;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        runUserCfg.user[i].enable = 0;
        memset(runUserCfg.user[i].userName, 0, sizeof(runUserCfg.user[i].userName));
        memset(runUserCfg.user[i].password, 0, sizeof(runUserCfg.user[i].password));
        runUserCfg.user[i].userRight = 0;
        runUserCfg.user[i].userLevel = 3;
    }

    adminMap[0].dataAddress = &(runUserCfg.user[0].enable);
    adminMap[1].dataAddress = &(runUserCfg.user[0].userName);
    adminMap[2].dataAddress = &(runUserCfg.user[0].password);
    adminMap[3].dataAddress = &(runUserCfg.user[0].userRight);
    adminMap[4].dataAddress = &(runUserCfg.user[0].userLevel);
    CfgLoadDefValue(adminMap);
    return 0;
}


int UserCfgLoad()
{
    int load_def_config = 0;
    char *data = NULL;

start_load:
    /* add by qqw */
    #if 0
    UserCfgLoadJson();
    #endif
    data = CfgReadFromFile(USER_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        PRINT_INFO("load %s error, so to load default cfg param.\n", USER_CFG_FILE);
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

    int i = 0;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
            char cjson_name[10] = {0};
            sprintf(cjson_name, "user_%d", i);
            adminMap[0].dataAddress = &(runUserCfg.user[i].enable);
            adminMap[1].dataAddress = &(runUserCfg.user[i].userName);
            adminMap[2].dataAddress = &(runUserCfg.user[i].password);
            adminMap[3].dataAddress = &(runUserCfg.user[i].userRight);
            adminMap[4].dataAddress = &(runUserCfg.user[i].userLevel);
            CfgParseCjson(json, cjson_name, adminMap);
    }

    adminMap[0].dataAddress = &(runUserCfg.user[0].enable);
    adminMap[1].dataAddress = &(runUserCfg.user[0].userName);
    adminMap[2].dataAddress = &(runUserCfg.user[0].password);
    adminMap[3].dataAddress = &(runUserCfg.user[0].userRight);
    adminMap[4].dataAddress = &(runUserCfg.user[0].userLevel);
    cJSON_Delete(json);
    free(data);
    return 0;

err:
    if(!load_def_config)
    {
        load_def_config = 1;
        char src_name[64], des_name[64];
        memset(src_name, 0, sizeof(src_name));
        snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, USER_CFG_FILE);
        memset(des_name, 0, sizeof(des_name));
        snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, USER_CFG_FILE);
        if(!copy_file(src_name, des_name))
            goto start_load;
    }
    UserCfgLoadDefValue();
    UserCfgSave();
    return 0;
}

int UserIsExist(const char *userName)
{
    int i;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        if (runUserCfg.user[i].enable == 1) {
            if (0 == strcmp(runUserCfg.user[i].userName, userName))
                return 1;
        }
    }

    return 0;
}

int UserAdd(GK_NET_USER_INFO *pUser)
{
    int is_exist = UserIsExist(pUser->userName);
    if (is_exist == 1)
        return -1;

    int i;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        if (runUserCfg.user[i].enable == 0) {
            strcpy(runUserCfg.user[i].userName, pUser->userName);
            strcpy(runUserCfg.user[i].password, pUser->password);
            if(pUser->userRight != -1)	// value comes from WEB or CMS or P2P
            	runUserCfg.user[i].userRight = pUser->userRight;
            if(pUser->userLevel != 0)	// value just comes from ONVIF
            	runUserCfg.user[i].userLevel = pUser->userLevel;
        	else
            	runUserCfg.user[i].userLevel = 3;	// if add user by web, set level to 3 (user level) for onvif
            runUserCfg.user[i].enable = 1;
            return 0;
        }
    }
    return -1;
}

int UserModify(GK_NET_USER_INFO *pUser)
{
    int i;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        if (runUserCfg.user[i].enable == 1) {
            if (0 == strcmp(runUserCfg.user[i].userName, pUser->userName)) {
                strcpy(runUserCfg.user[i].password, pUser->password);
				if(pUser->userRight != -1)	// value comes from WEB or CMS or P2P
	                runUserCfg.user[i].userRight = pUser->userRight;
				if(pUser->userLevel != 0)	// value just comes from ONVIF
                	runUserCfg.user[i].userLevel = pUser->userLevel;
                return 0;
            }   
        }
    }
    return -1;
}

int UserCheck(GK_NET_USER_INFO *pUser)
{
    int i;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        if (runUserCfg.user[i].enable == 1) {
            if ((0 == strcmp(runUserCfg.user[i].userName, pUser->userName)) &&
               (0 == strcmp(runUserCfg.user[i].password, pUser->password))) {
                    return 0;
            }
        }
    }
    return -1;
}

int UserModifyByIndex(int index, GK_NET_USER_INFO *pUser)
{
    strcpy(runUserCfg.user[index].userName, pUser->userName);
    strcpy(runUserCfg.user[index].password, pUser->password);
	if(pUser->userRight != -1)	// value comes from WEB or CMS or P2P
    	runUserCfg.user[index].userRight = pUser->userRight;
	if(pUser->userRight != 0)	// value just comes from ONVIF
    	runUserCfg.user[index].userLevel = pUser->userLevel;
    runUserCfg.user[index].enable = 1;

    return 0;
}

int UserDel(const char *userName)
{
    int i;
    for (i = 0; i < GK_MAX_USER_NUM; i ++) {
        if (runUserCfg.user[i].enable == 1) {
            if (strcmp(runUserCfg.user[i].userName, userName) == 0) {
                runUserCfg.user[i].enable = 0;
                memset(runUserCfg.user[i].userName, 0, sizeof(runUserCfg.user[i].userName));
                memset(runUserCfg.user[i].password, 0, sizeof(runUserCfg.user[i].password));
                runUserCfg.user[i].userRight = 0;
                runUserCfg.user[i].userLevel = 0;
                return 0;
            }
        }
    }
    return -1;
}


char* UserCfgLoadJson( void )
{
	cJSON* ArrayObject = cJSON_CreateArray();
	char *data = NULL;
    int i = 0;
    for (i = 0; i < GK_MAX_USER_NUM; i ++)
	{
        if (runUserCfg.user[i].enable == 1)
		{
			cJSON *item = cJSON_CreateObject();

		    cJSON_AddItemToObject(item, "userName", cJSON_CreateString(runUserCfg.user[i].userName));
		    cJSON_AddItemToObject(item, "password", cJSON_CreateString(runUserCfg.user[i].password));
		    //cJSON_AddItemToObject(item, "userRight", cJSON_CreateNumber(runUserCfg.user[i].userRight));
		    cJSON_AddItemToObject(item, "userRight", cJSON_CreateString("11111"));
			cJSON_AddItemToArray(ArrayObject, item );


		    //cJSON_Delete(item);
        }
    }

	data = cJSON_Print(ArrayObject);
    cJSON_Delete(ArrayObject);
	PRINT_INFO("UserCfgLoadJson:%s\n", data);
    return data;
}

int UserCfgSaveJson( char *data)
{

	cJSON* Object = cJSON_Parse(data);
	int Size = cJSON_GetArraySize(Object);
	int states=-1;
	cJSON* Item = cJSON_GetArrayItem(Object,0);
	GK_NET_USER_INFO user;
	memset(&user, 0, sizeof(GK_NET_USER_INFO));
	user.enable = 1;
	user.userRight = 2;
	strcpy(user.userName, cJSON_GetObjectItem(Item,"userName")->valuestring);
	strcpy(user.password, cJSON_GetObjectItem(Item,"password")->valuestring);

	if(Size==2)
	{
		if(UserCheck(&user)==0)
		{
			strcpy(user.password, cJSON_GetObjectItem(cJSON_GetArrayItem(Object,1), "password")->valuestring);
			UserModify(&user);
			states=1;
		}
		else
			states=2;
	}
	else
	{
		if(UserAdd(&user)==-1)
		{
			UserDel(user.userName);
		}
		states=1;
	}

    cJSON_Delete(Object);
	UserCfgSave();
	return states;
}

int UserMatching_login( char *username, char *password)
{
	int states=0;
	GK_NET_USER_INFO user;
	user.enable = 1;
	user.userRight = 2;
	strcpy(user.userName, username);
	strcpy(user.password, password);

	if(UserCheck(&user)==0)
	{
		states=1;
	}
	return states;
}


/*#define USER_CJSON_FILE "gk_user.cjson"

char* UserCfgLoadJson( void )
{
	cJSON* ArrayObject;
	char *data = NULL;
	char path[64]={0};
    sprintf(path, "%s%s", CFG_DIR, USER_CJSON_FILE);
	// check file whether exist.
	//  exist, return 0
	//  net exist, return -1;
	//
	if(access(path, F_OK) < 0)
	{
		ArrayObject = cJSON_CreateArray();
		cJSON_AddItemToArray(ArrayObject, cJSON_Parse("{\n\"userName\":\"admin\",\n\"password\":\"\",\n\"userRight\":\"1111\"\n}") );
		cJSON_WriteFile( path, ArrayObject);
	}
	else
	{
		ArrayObject = cJSON_ReadFile(path);
	}
	data = cJSON_Print(ArrayObject);
    cJSON_Delete(ArrayObject);
	PRINT_INFO("ReadFile:%s\n%s\n", path, data);
    return data;
}

int UserCfgSaveJson( char *data)
{
	char path[64]={0};
    sprintf(path, "%s%s", CFG_DIR, USER_CJSON_FILE);
	cJSON* ArrayObject = cJSON_ReadFile(path);
	cJSON* Object = cJSON_Parse(data);
    cJSON* Item = cJSON_GetArrayItem(Object,0);

	int ArraySize = cJSON_GetArraySize(ArrayObject);
	int Size = cJSON_GetArraySize(Object);
	int i=0,states=-1, exist=0;

	for(i=0;i<ArraySize;i++)
	{
		if(strcmp(cJSON_GetObjectItem(Item,"userName")->valuestring, cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"userName")->valuestring) == 0)
		{
			if(Size==1)
			{
				if( strcmp(cJSON_GetObjectItem(Item,"password")->valuestring,  "") == 0 && strcmp(cJSON_GetObjectItem(Item,"userRight")->valuestring,  "0000") == 0 )//删除
				{
					cJSON_DeleteItemFromArray(ArrayObject, i);
					states=1;
					break;
				}
				else//增加用户有相同的情况
				{
					exist=1;
					states=0;
					break;
				}
			}
			else
			{
				if(strcmp(cJSON_GetObjectItem(Item,"password")->valuestring, cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"password")->valuestring) == 0)//修改
				{
					Item = cJSON_GetArrayItem(Object,1);
					strcpy(cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"password")->valuestring, cJSON_GetObjectItem(Item,"password")->valuestring);
					strcpy(cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"userRight")->valuestring, cJSON_GetObjectItem(Item,"userRight")->valuestring);
					states=1;
					break;
				}
				else//修改时密码错误
				{
					states=2;
					break;
				}
			}
		}
	}
	if(exist==0&&i==ArraySize&&Size==1)//zengjia
	{
		cJSON_AddItemToArray(ArrayObject, cJSON_GetArrayItem(Object,0) );
		states=1;
	}
	PRINT_INFO("WriteFile:%s\n%s\n", path, cJSON_Print(ArrayObject) );
	cJSON_WriteFile( path, ArrayObject);
    cJSON_Delete(ArrayObject);
	return states;
}

int UserMatching_login( char *username, char *password)
{
	char path[64]={0};
    	sprintf(path, "%s%s", CFG_DIR, USER_CJSON_FILE);
	cJSON* ArrayObject = cJSON_ReadFile(path);

	int ArraySize = cJSON_GetArraySize(ArrayObject);
	int i=0;
	int states=0;

	for(i=0;i<ArraySize;i++)
	{
		if(strcmp(username, cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"userName")->valuestring) == 0&&strcmp(password, cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"password")->valuestring) == 0)
		{
			states=1;
			break;
		}
	}

  	cJSON_Delete(ArrayObject);
	return states;
}*/
/*
char* UserRight_Get(char *username)
{
	char path[64]={0};
	sprintf(path, "%s%s", CFG_DIR, USER_CJSON_FILE);
	cJSON* ArrayObject = cJSON_ReadFile(path);
	if(ArrayObject == NULL)
	{
		return 0;
	}
	int ArraySize = cJSON_GetArraySize(ArrayObject);
	int i=0;
	char right[32]={0};
	for(i=0;i<ArraySize;i++)
	{
		if(strcmp(username, cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"userName")->valuestring))
		{
			strcpy(right,cJSON_GetObjectItem(cJSON_GetArrayItem(ArrayObject,i),"userRight")->valuestring);
		}
	}
	cJSON_Delete(ArrayObject);
	return right;
}
*/

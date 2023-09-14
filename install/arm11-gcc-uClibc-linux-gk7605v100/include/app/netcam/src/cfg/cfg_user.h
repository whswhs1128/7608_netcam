/*!
*****************************************************************************
** FileName     : cfg_md.h
**
** Description  : config for motion detect.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_USER_H__
#define _GK_CFG_USER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***         user                ***/
/***********************************/
#define GK_MAX_USER_NUM             8 
typedef struct {
    SDK_S32   enable;  /*0:无效用户，禁用, 1:启用*/
    SDK_S8    userName[MAX_STR_LEN_64]; /* 用户名最大32字节*/
    SDK_S8    password[MAX_STR_LEN_64]; /* 密码 */
    SDK_U32   userRight; /* 权限，对应掩码决定 ，16 项 */ // if userRight == -1, the value is from ONVIF, and it should be igored.
    SDK_S32   userLevel; /* to adapt to user requirement in onvif, 1: Administrator, 2: Operator, 3: User, 4: Anonymous, 5: Extended*/
} GK_NET_USER_INFO,*LPGK_NET_USER_INFO;

typedef struct {
    GK_NET_USER_INFO user[GK_MAX_USER_NUM]; //最大支持32个用户
} GK_NET_USER_CFG,*LPGK_NET_USER_CFG;

extern int UserCfgSave();
extern int UserCfgLoad();
extern void UserCfgPrint();
extern int UserCfgLoadDefValue();

int UserIsExist(const char *userName);
int UserAdd(GK_NET_USER_INFO *pUser);
int UserModify(GK_NET_USER_INFO *pUser);
int UserModifyByIndex(int i, GK_NET_USER_INFO *pUser);
int UserDel(const char *userName);
int UserCheck(GK_NET_USER_INFO *pUser);

extern char* UserCfgLoadJson(void);
extern int UserCfgSaveJson(char *data);
int UserMatching_login( char *username, char *password);

#define USER_CFG_FILE "gk_user_cfg.cjson"

extern GK_NET_USER_CFG runUserCfg;

#ifdef __cplusplus
}
#endif
#endif


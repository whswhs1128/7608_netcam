/*!
*****************************************************************************
** FileName     : phone_mtws.h
**
** Description  : api for mtws app.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2016-4-8
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_PHONE_MTWS_H__
#define _GK_PHONE_MTWS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cloud_if.h"

typedef struct tagMTWS_CONFIG
{
    unsigned int crc32_val;  /* crc32 校验值  */
    int size;
    char ap_name[32];      /* ap name         */
    char lan_mac_addr[20];   /* lan mac 地址          */
    char wifi_mac_addr[20];  /* wifi mac 地址          */
    char manufacturer_id[32]; /*厂家id*/
    char reserve[1024];       /* 保留位           */
} MTWS_CONFIG;

typedef enum tagMSG_CMD_TYPE
{
	CMD_CONNECT_SSID = 0,
	
} MSG_CMD_TYPE;

typedef enum tagUPDATE_RESULT_TYPE
{
    UPDATE_OK = 0,
    UPDATE_JSON_ERR = -1,
    UPDATE_FILE_ERR = -2,
    UPDATE_MEM_ERR = -3,
    UPDATE_MD5_ERR = -4,
    UPDATE_UP_ERR = -5,
    UPDATE_VERSION_ERR = -6,
} UPDATE_RESULT_TYPE;

typedef struct tagMTWS_LOCAL_MSG_HEAD
{
	MSG_CMD_TYPE cmd;
    int size;
} MTWS_LOCAL_MSG_HEAD;


#define MTWS_AP_NAME "12345678"
#define SSID_CONF "/opt/custom/cfg/tkip.conf"
#define MTWS_LOCAL_TCP_LISTEN_PORT 0x9126
#define TEST_IP "www.baidu.com"
#define WIFI_APT_NAME "ra0"


extern int mtws_app_init();
extern void mtws_app_exit();

#ifdef __cplusplus
}
#endif
#endif

/*!
*****************************************************************************
** FileName     : cfg_system.h
**
** Description  : config for device infomation.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_SYSTEM_H__
#define _GK_CFG_SYSTEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/******************************************/
/*** device_info, time, zone, dst, NTP  ***/

/******************************************/

#define INVALID_TIEM_ZONE  0XFFFF

typedef enum {
    GK_PAL = 0,
    GK_NTSC,
} GK_VIDEO_STANDARD_E;

typedef struct {
    SDK_S8    deviceName[MAX_SYSTEM_STR_SIZE];
    SDK_S8    manufacturer[MAX_SYSTEM_STR_SIZE];
    SDK_S8    deviceType[MAX_SYSTEM_STR_SIZE];
    SDK_S32   sensorType; // 0: IMX222; 1 OV9710 2 ...
    SDK_S32   languageType;   // 0: ����; 1 Ӣ�� 2 ...
    GK_VIDEO_STANDARD_E videoType; //pal, ntsc
    SDK_S32   odmId;   // 1: ��˴; 10 ����
    SDK_S8    model[MAX_STR_LEN_16];
	SDK_S32   devType;
   //���²����޸�
    SDK_S8    serialNumber[MAX_SYSTEM_STR_SIZE];//���к�
    SDK_S8    softwareVersion[MAX_SYSTEM_STR_SIZE]; //����汾�ţ���16λ�����汾����16λ�Ǵΰ汾
    SDK_S8    softwareBuildDate[MAX_SYSTEM_STR_SIZE]; //����������ڣ�0xYYYYMMDD
    SDK_S8    firmwareVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    firmwareReleaseDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8    hardwareVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    hardwareBuildDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8    webVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    webBuildDate[MAX_SYSTEM_STR_SIZE];
    SDK_S8    upgradeVersion[MAX_SYSTEM_STR_SIZE];
    SDK_S8    upgradestatus;
} GK_NET_DEVICE_INFO;

/*ʱ������ʱ�Ʋ���*/
typedef struct  {
    SDK_U32 month;    //�� 0-11��ʾ1-12����
    SDK_U32 weekNo;   //�ڼ��ܣ�0����1�ܣ�1����2�ܣ�2����3�ܣ� 3����4�ܣ�4�����һ��
    SDK_U32 weekDate; //���ڼ���0�������գ�1������һ��2�����ڶ���3����������4�������ģ�5�������壻6��������
    SDK_U32 hour;     //Сʱ����ʼʱ��ȡֵ��Χ0��23�� ����ʱ��ȡֵ��Χ1��23
    SDK_U32 min;      //��0��59
} GK_NET_TIMEPOINT;

typedef enum {
    GK_GMT_NEG_12 = -12,
    GK_GMT_NEG_11 = -11,
    GK_GMT_NEG_10 = -10,
    GK_GMT_NEG_9  = -9,
    GK_GMT_NEG_8  = -8,
    GK_GMT_NEG_7  = -7,
    GK_GMT_NEG_6  = -6,
    GK_GMT_NEG_5  = -5,
    GK_GMT_NEG_4  = -4,
    GK_GMT_NEG_3  = -3,
    GK_GMT_NEG_2  = -2,
    GK_GMT_NEG_1  = -1,
    GK_GMT_0      = 0,
    GK_GMT_POS_1  = 1,
    GK_GMT_POS_2  = 2,
    GK_GMT_POS_3  = 3,
    GK_GMT_POS_4  = 4,
    GK_GMT_POS_5  = 5,
    GK_GMT_POS_6  = 6,
    GK_GMT_POS_7  = 7,
    GK_GMT_POS_8  = 8,
    GK_GMT_POS_9  = 9,
    GK_GMT_POS_10 = 10,
    GK_GMT_POS_11 = 11,
    GK_GMT_POS_12 = 12,
}GK_TimeZone_E;

typedef struct {
    int     timezone;   //-720, +720  ʱ�� ����,
	SDK_S8  time_desc[256];
} GK_NET_ZONE, *LPGK_NET_ZONE;

typedef struct {
    SDK_S32    enableDST;       //�Ƿ�������ʱ�� 0�������� 1������
    SDK_S32    dSTBias;         //����ʱƫ��ֵ��30min, 60min, 90min, 120min, �Է��Ӽƣ�����ԭʼ��ֵ
    SDK_S8     beginTime[MAX_TIME_STR_SIZE]; //��ʱ�ƿ�ʼʱ��
    SDK_S8     endTime[MAX_TIME_STR_SIZE];   //��ʱ��ֹͣʱ��
} GK_NET_DST, *LPGK_NET_DST;

typedef struct {
    SDK_S32   enable;
    SDK_S8    serverDomain[MAX_URL_STR_SIZE];
    SDK_S32   sync_time;
    SDK_S32   port;
    SDK_S8      platurl[64];
    SDK_S32	    enable485;
    SDK_S32         enableAI;
    SDK_S8         modelNum[100];
    SDK_S8          AIplat[64];
    SDK_S8          AI_str[64];
} GK_NTP_CFG;

typedef struct {
    SDK_U8      enable;
    SDK_U8      index; /* 0 ������ ... 6 ������ 7 ÿ��*/
    SDK_U8      hour;
    SDK_U8      minute;
    SDK_U8      second;
    SDK_S32     newest_time;
} GK_MAINTAIN_CFG;

typedef struct {
    GK_NET_DEVICE_INFO deviceInfo;
    GK_NET_ZONE timezoneCfg;
    GK_NET_DST netDstCfg;
    GK_NTP_CFG ntpCfg;
	GK_MAINTAIN_CFG maintainCfg;
} GK_NET_SYSTEM_CFG;

/*
  add by heyong, for load special system info.
*/
typedef struct {
    SDK_S8    device_name[MAX_SYSTEM_STR_SIZE];
    SDK_S8    device_type[MAX_SYSTEM_STR_SIZE];
    SDK_S8    chip_type[MAX_SYSTEM_STR_SIZE];
    SDK_S8    sensor_type[MAX_SYSTEM_STR_SIZE];
    SDK_S8    svn_version[MAX_SYSTEM_STR_SIZE]; //YYYYMMDD
    SDK_S8    make_date[MAX_SYSTEM_STR_SIZE];
    SDK_S8    upgrade_version[MAX_SYSTEM_STR_SIZE]; //YYYYMMDD
} GK_SYSTEM_INFO;


extern int SystemCfgSave();
extern int SystemCfgLoad();
extern void SystemCfgPrint();
extern int SystemCfgLoadDefValue();
extern char *SytemCfgGetCjsonString();
extern cJSON *SystemCfgGetNTPJsonSting();


#define SYSTEM_INFO_FILE "/root/sys_info"
#define SYSTEM_CFG_FILE "gk_system_cfg.cjson"

extern GK_SYSTEM_INFO g_systemInfo;
extern GK_NET_SYSTEM_CFG runSystemCfg;
extern GK_CFG_MAP deviceInfoMap[];
extern GK_CFG_MAP timezoneCfgMap[];
extern GK_CFG_MAP netDstCfgCfgMap[];
extern GK_CFG_MAP ntpCfgMap[];

#ifdef __cplusplus
}
#endif
#endif


/*!
*****************************************************************************
** FileName     : cfg_shelter.h
**
** Description  : config for shelter.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_SHELTER_H__
#define _GK_CFG_SHELTER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"


/***********************************/
/***        shelter            ***/
/***********************************/
//�ڵ���������
typedef struct {
    SDK_S32    enable;     /* �ڵ�ʹ�� 0-����, 1-ʹ�� */
    float    x;          /* �ڵ��������Ͻǵ�x���� */
    float    y;          /* �ڵ��������Ͻǵ�y���� */
    float    width;      /* �ڵ������� */
    float    height;     /* �ڵ�����߶� */
    SDK_U32    color;      /* �ڵ���ɫ, Ĭ�� 0:��ɫ ��RGB��ʽ*/
} GK_NET_SHELTER_RECT;

/***********************************/
/***         osd                 ***/
/***********************************/
typedef struct {
    SDK_S32   enable;            /* 0-����, 1-ʹ�� */
    SDK_S8    text[MAX_STR_LEN_128];  //ͨ������
    float   x;
    float   y;
} GK_NET_OSD_CHANNEL_NAME;

typedef struct {
    SDK_S32    enable; /* 0-����, 1-ʹ�� */
    SDK_U8     dateFormat;
                /*���ڸ�ʽ(�����ո�ʽ)��
				0��YYYY-MM-DD		������	
				1��MM-DD-YYYY		������ 
				2��YYYY/MM/DD		������ 
				3��MM/DD/YYYY		������	
				4��DD-MM-YYYY		������ 
				5��DD/MM/YYYY		������

                */
    SDK_U8     dateSprtr; //0��":"��1��"-"��2��"/" 3��"."
    SDK_U8     timeFmt; //ʱ���ʽ (0-24Сʱ��1��12Сʱ).
    float    x;
    float    y;
	SDK_U8		displayWeek; //�Ƿ���ʾ���ڼ�
} GK_NET_OSD_DATETIME;

typedef struct {    
	SDK_S32   enable;            /* 0-����, 1-ʹ�� */    
	SDK_S8    text[MAX_STR_LEN_128];  //ͨ������    
	float   x;    
	float   y;
} GK_NET_OSD_CHANNEL_ID;


/***********************************/
/***    channel                  ***/
/***********************************/

typedef struct {
    SDK_S32                id;

    GK_NET_OSD_CHANNEL_NAME osdChannelName;
    GK_NET_OSD_DATETIME     osdDatetime;
	GK_NET_OSD_CHANNEL_ID	osdChannelID;

} GK_NET_CHANNEL_INFO, *LPGK_NET_CHANNEL_INFO;

typedef struct {
    GK_NET_CHANNEL_INFO channelInfo[4];
    GK_NET_SHELTER_RECT shelterRect[4]; //֧��4���ڵ�����
} GK_NET_CHANNEL_CFG, *LPGK_NET_CHANNEL_CFG;

extern int ChannelCfgSave();
extern int ChannelCfgLoad();
extern void ChannelCfgPrint();
extern int ChannelCfgLoadDefValue();
extern char* ChannelCfgLoadOverlayJson( int channelId);
extern char* ChannelCfgLoadCoverJson( int channelId);

#define CHANNEL_CFG_FILE "gk_channel_cfg.cjson"

#define MAX_CHANNEL_NUM 4
#define MAX_CHANNEL_ITEM_NUM 17
#define MAX_SHELTER_RECT_ITEM_NUM 7

extern GK_NET_CHANNEL_CFG runChannelCfg;
extern GK_CFG_MAP osdMap[MAX_CHANNEL_NUM][MAX_CHANNEL_ITEM_NUM];
extern GK_CFG_MAP shelterRectMap[MAX_CHANNEL_NUM][MAX_SHELTER_RECT_ITEM_NUM];

#ifdef __cplusplus
}
#endif
#endif


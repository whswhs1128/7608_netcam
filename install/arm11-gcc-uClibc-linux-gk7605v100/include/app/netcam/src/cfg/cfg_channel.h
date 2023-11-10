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
//遮挡区域设置
typedef struct {
    SDK_S32    enable;     /* 遮挡使能 0-禁用, 1-使能 */
    float    x;          /* 遮挡区域左上角的x坐标 */
    float    y;          /* 遮挡区域左上角的y坐标 */
    float    width;      /* 遮挡区域宽度 */
    float    height;     /* 遮挡区域高度 */
    SDK_U32    color;      /* 遮挡颜色, 默认 0:黑色 按RGB格式*/
} GK_NET_SHELTER_RECT;

/***********************************/
/***         osd                 ***/
/***********************************/
typedef struct {
    SDK_S32   enable;            /* 0-禁用, 1-使能 */
    SDK_S8    text[MAX_STR_LEN_128];  //通道名称
    float   x;
    float   y;
} GK_NET_OSD_CHANNEL_NAME;

typedef struct {
    SDK_S32    enable; /* 0-禁用, 1-使能 */
    SDK_U8     dateFormat;
                /*日期格式(年月日格式)：
				0－YYYY-MM-DD		年月日	
				1－MM-DD-YYYY		月日年 
				2－YYYY/MM/DD		年月日 
				3－MM/DD/YYYY		月日年	
				4－DD-MM-YYYY		日月年 
				5－DD/MM/YYYY		日月年

                */
    SDK_U8     dateSprtr; //0：":"，1："-"，2："/" 3："."
    SDK_U8     timeFmt; //时间格式 (0-24小时，1－12小时).
    float    x;
    float    y;
	SDK_U8		displayWeek; //是否显示星期几
} GK_NET_OSD_DATETIME;

typedef struct {    
	SDK_S32   enable;            /* 0-禁用, 1-使能 */    
	SDK_S8    text[MAX_STR_LEN_128];  //通道名称    
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
    GK_NET_SHELTER_RECT shelterRect[4]; //支持4个遮挡区域
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


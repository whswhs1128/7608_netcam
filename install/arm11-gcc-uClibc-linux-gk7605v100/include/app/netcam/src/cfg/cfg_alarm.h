/*!
*****************************************************************************
** FileName     : cfg_alarm.h
**
** Description  : config for alarm in, alarm out.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_ALARM_H__
#define _GK_CFG_ALARM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"
#include "cfg_ptz.h"

/* 消息处理方式，可以同时多种处理方式，包括
* 0x00000000 - 无响应
* 0x00000001 - 报警上传中心
* 0x00000002 - 联动录象
* 0x00000004 - 云台联动
* 0x00000008 - 发送Email邮件，根据email配置是否带附件是否有效，决定是否图片。
* 0x00000010 - 本地轮巡（该版本不支持）
* 0x00000020 - 本地提示，监视器上警告
* 0x00000040 - 报警输出
* 0x00000080 - Ftp抓图上传
* 0x00000100 - 蜂鸣
* 0x00000200 - 语音提示
* 0x00000400 - 抓图本地保存
* 0x00000800 - 主动请求对讲
* FTP,EMAIL,抓图本地保存三个功能的抓图通道都是根据结构体成员bySnap决定。
* 如果bySnap没有指定抓图通道，则FTP，抓图本地保存功能失效；EMAIL只发送文本信息，不带图片附件。
*/
typedef enum {
    GK_ALARM_EXCEPTION_NORESPONSE = 0x00000000,
    GK_ALARM_EXCEPTION_UPTOCENTER = 0x00000001,
    GK_ALARM_EXCEPTION_TOREC = 0x00000002,
    GK_ALARM_EXCEPTION_TOPTZ = 0x00000004,
    GK_ALARM_EXCEPTION_TOEMAIL = 0x00000008,
    GK_ALARM_EXCEPTION_TOPOLL = 0x00000010,
    GK_ALARM_EXCEPTION_TOSCREENTIP = 0x00000020,
    GK_ALARM_EXCEPTION_TOALARMOUT = 0x00000040,
    GK_ALARM_EXCEPTION_TOFTP = 0x00000080,
    GK_ALARM_EXCEPTION_TOBEEP = 0x00000100,
    GK_ALARM_EXCEPTION_TOVOICE = 0x00000200,
    GK_ALARM_EXCEPTION_TOSNAP = 0x00000400,
    GK_ALARM_EXCEPTION_TOTALK = 0x00000800,
} GK_ALARM_EXCEPTION_TO;


/***********************************/
/***         alarm               ***/
/***********************************/
// 报警联动结构体
#pragma pack(4)
typedef struct {
    SDK_U8   enable;
    SDK_U32  x;
    SDK_U32  y;
    SDK_U32  width;
    SDK_U32  height;
    SDK_U8   sensitivity;
    SDK_U8   regionTime;
    SDK_U8   regionRatio;
} GK_ALARM_RECT;

typedef struct {
    SDK_U8   enable;
    SDK_U32  startX;
    SDK_U32  startY;
    SDK_U32  endX;
    SDK_U32  endY;
    SDK_U8   direction;
} GK_ALARM_LINE;

/* 人形检测*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_RECT      rect[4]; //支持4个区域
} GK_ALARM_HD_CFG, *LPGK_ALARM_HD_CFG;

/* 运动追踪*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_RECT      rect[4]; //支持4个区域
    SDK_U8             retPoint;// 通过设置预置点实现
    SDK_U32            retTime; // 返回时间
} GK_ALARM_MT_CFG, *LPGK_ALARM_MT_CFG;

/* 越界侦测*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_LINE      line[4]; //支持4条越界线
    SDK_S32            schedule_mode;     //1(默认值)按布防时间scheduleTime, scheduleSlice   0全时段检测
    GK_SCHEDTIME       scheduleTime[7][4];  /*该通道的布防时间*/
    SDK_U32            scheduleSlice[7][3]; /*该通道的布防时间片段*/
} GK_ALARM_TD_CFG, *LPGK_ALARM_TD_CFG;

/* 区域入侵*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_RECT      rect[4]; //支持4个区域
    SDK_S32            schedule_mode;     //1(默认值)按布防时间scheduleTime, scheduleSlice   0全时段检测
    GK_SCHEDTIME       scheduleTime[7][4];  /*该通道的布防时间*/
    SDK_U32            scheduleSlice[7][3]; /*该通道的布防时间片段*/
} GK_ALARM_RI_CFG, *LPGK_ALARM_RI_CFG;


typedef struct {
    SDK_U32               intervalTime;   /* 报警间隔时间，s为单位*/

    /* 联动发送email */
    SDK_U8                is_email;

    /* 联动录象 */
    SDK_U8                is_rec;
    SDK_U32               recTime;        /* 联动录象持续时间,分钟 */
    SDK_U32               recStreamNo;    /* 用哪一路码流来录像 */

    /* 联动抓图 */
    SDK_U8                is_snap;
    SDK_U8                isSnapUploadToFtp;      /* 是否上传 */
    SDK_U8                isSnapUploadToWeb;      /* 是否上传 */
    SDK_U8                isSnapUploadToCms;      /* 是否上传 */
    SDK_U8                isSnapSaveToSd;  /* 是否保存在SD卡上 */
    SDK_U32               snapNum;    /* 连续抓拍图片数量*/
    SDK_U32 	          interval;   /* 连拍间隔,单位毫秒 */

    /* 联动报警输出 */
    SDK_U8                is_alarmout;
    SDK_U32               duration;   /* 报警输出持续时间，秒*/

    /* 联动蜂鸣器 */
    SDK_U8                is_beep;   /* 是否蜂鸣器鸣叫 */
    SDK_U32               beepTime;  /* 蜂鸣器输出时间，秒*/

    /* 联动云台输出 */
    SDK_U8                is_ptz;
    SDK_U8                is_pushMsg;

    GK_PTZ_LINK           ptzLink;

    SDK_U8                alarmAudioType; /* 报警声音类型*/
    SDK_U8                is_light[4];       /* 白光灯*/
    SDK_U8                bCheckedFigure; /* 人形检测*/
    SDK_U8                colorModeTime; /* 智能夜视切换周期，秒*/

    GK_ALARM_HD_CFG       humanDetection;    /* 人形检测*/
    GK_ALARM_MT_CFG       motionTracing;     /* 运动追踪*/
	GK_ALARM_TD_CFG       transboundaryDetection;     /* 越界侦测*/
    GK_ALARM_RI_CFG       regionIntrusion;     /* 区域入侵*/
} GK_HANDLE_EXCEPTION;


/*报警输入*/
typedef struct {
    SDK_S32            channel;
    SDK_S8             alarmInName[MAX_STR_LEN_32];    /* 名称 */
    SDK_S8             defaultState;       /* 0-low 1-high */
    SDK_S8             activeState;        /* 0-low 1-high*/
    GK_HANDLE_EXCEPTION handle;               /* 处理方式 */
    GK_SCHEDTIME        scheduleTime[7][4];   /**该通道的videoloss的布防时间*/
} GK_NET_ALARMINCFG, *LPGK_NET_ALARMINCFG;

/*报警输出*/
typedef struct {
    SDK_S32   channel;
    SDK_S8    alarmOutName[MAX_STR_LEN_32];   /* 名称 */
    SDK_S8    defaultState;       /* 0-low 1-high */
    SDK_S8    activeState;        /* 0-low 1-high*/
    SDK_S8    powerOnState;       /* 0-pulse 1-continuous*/
    SDK_S32   pulseDuration;      /* 1000 - 10000*/
    GK_SCHEDTIME scheduleTime[7][4]; /**该通道的videoloss的布防时间*/
} GK_NET_ALARMOUTCFG, *LPGK_NET_ALARMOUTCFG;

/*报警配置*/
typedef struct {
	SDK_S8  gpio_enable;
	SDK_S8  in_level;
	SDK_S8  out_level;
	SDK_S8  io_linkage;

	SDK_S8 upload_interval;
	SDK_S8 alarmbymail;
	SDK_S8 schedule_enable;
	SDK_S8 snapshot_enable;
	SDK_S8 recorder_enable;
	SDK_S8 move_perset;
	SDK_S8 alarmMute;
} GK_ALARMCFG;

/*魔镜4.0配置*/
typedef struct {
	SDK_S8  gbStatus;   //国标告警消息输出开关状态（1：开，2：关）
	SDK_S8  audioAlarmStatus;   //声音告警开关状态（1：开，2：关，-1:不支持）
	SDK_S8  audioFilesCur[128];
	SDK_S8  audioPlayCnt;

	SDK_S8 lightAlarmStatus;    //告警灯光开关状态（1：开，2：关，-1:不支持）
	SDK_S8 lightAlarmType;      //灯光告警类型，1=常亮，2=闪烁
	SDK_S8 lightAlarmFrequency; //闪烁频率，1=高，2=中，3=低
	SDK_S8 lightAlarmTime;      //停留时间（单位s）（范围5-30s）
} GK_MOJING_ALRAM;

typedef struct {
    GK_NET_ALARMINCFG  alarmIn;
    GK_NET_ALARMOUTCFG alarmOut;
	GK_ALARMCFG        cfg;
    
	SDK_S8              audioFiles[384];
    GK_MOJING_ALRAM     alarmInfo[2]; //0:移动侦测 1:区域入侵
} GK_NET_ALARM_CFG, *LPGK_NET_ALARM_CFG;

#pragma pack()

extern int AlarmCfgSave();
extern int AlarmCfgLoad();
extern void AlarmCfgPrint();
extern int AlarmCfgLoadDefValue();

#define ALARM_CFG_FILE "gk_alarm_cfg.cjson"

extern GK_NET_ALARM_CFG runAlarmCfg;
extern GK_CFG_MAP alarmInMap[];
extern GK_CFG_MAP alarmOutMap[];

#ifdef __cplusplus
}
#endif
#endif


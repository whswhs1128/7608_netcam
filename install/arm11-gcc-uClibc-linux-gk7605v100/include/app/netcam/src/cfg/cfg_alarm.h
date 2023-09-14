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

/* ��Ϣ����ʽ������ͬʱ���ִ���ʽ������
* 0x00000000 - ����Ӧ
* 0x00000001 - �����ϴ�����
* 0x00000002 - ����¼��
* 0x00000004 - ��̨����
* 0x00000008 - ����Email�ʼ�������email�����Ƿ�������Ƿ���Ч�������Ƿ�ͼƬ��
* 0x00000010 - ������Ѳ���ð汾��֧�֣�
* 0x00000020 - ������ʾ���������Ͼ���
* 0x00000040 - �������
* 0x00000080 - Ftpץͼ�ϴ�
* 0x00000100 - ����
* 0x00000200 - ������ʾ
* 0x00000400 - ץͼ���ر���
* 0x00000800 - ��������Խ�
* FTP,EMAIL,ץͼ���ر����������ܵ�ץͼͨ�����Ǹ��ݽṹ���ԱbySnap������
* ���bySnapû��ָ��ץͼͨ������FTP��ץͼ���ر��湦��ʧЧ��EMAILֻ�����ı���Ϣ������ͼƬ������
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
// ���������ṹ��
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

/* ���μ��*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_RECT      rect[4]; //֧��4������
} GK_ALARM_HD_CFG, *LPGK_ALARM_HD_CFG;

/* �˶�׷��*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_RECT      rect[4]; //֧��4������
    SDK_U8             retPoint;// ͨ������Ԥ�õ�ʵ��
    SDK_U32            retTime; // ����ʱ��
} GK_ALARM_MT_CFG, *LPGK_ALARM_MT_CFG;

/* Խ�����*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_LINE      line[4]; //֧��4��Խ����
    SDK_S32            schedule_mode;     //1(Ĭ��ֵ)������ʱ��scheduleTime, scheduleSlice   0ȫʱ�μ��
    GK_SCHEDTIME       scheduleTime[7][4];  /*��ͨ���Ĳ���ʱ��*/
    SDK_U32            scheduleSlice[7][3]; /*��ͨ���Ĳ���ʱ��Ƭ��*/
} GK_ALARM_TD_CFG, *LPGK_ALARM_TD_CFG;

/* ��������*/
typedef struct {
    SDK_U8             enable;
    GK_ALARM_RECT      rect[4]; //֧��4������
    SDK_S32            schedule_mode;     //1(Ĭ��ֵ)������ʱ��scheduleTime, scheduleSlice   0ȫʱ�μ��
    GK_SCHEDTIME       scheduleTime[7][4];  /*��ͨ���Ĳ���ʱ��*/
    SDK_U32            scheduleSlice[7][3]; /*��ͨ���Ĳ���ʱ��Ƭ��*/
} GK_ALARM_RI_CFG, *LPGK_ALARM_RI_CFG;


typedef struct {
    SDK_U32               intervalTime;   /* �������ʱ�䣬sΪ��λ*/

    /* ��������email */
    SDK_U8                is_email;

    /* ����¼�� */
    SDK_U8                is_rec;
    SDK_U32               recTime;        /* ����¼�����ʱ��,���� */
    SDK_U32               recStreamNo;    /* ����һ·������¼�� */

    /* ����ץͼ */
    SDK_U8                is_snap;
    SDK_U8                isSnapUploadToFtp;      /* �Ƿ��ϴ� */
    SDK_U8                isSnapUploadToWeb;      /* �Ƿ��ϴ� */
    SDK_U8                isSnapUploadToCms;      /* �Ƿ��ϴ� */
    SDK_U8                isSnapSaveToSd;  /* �Ƿ񱣴���SD���� */
    SDK_U32               snapNum;    /* ����ץ��ͼƬ����*/
    SDK_U32 	          interval;   /* ���ļ��,��λ���� */

    /* ����������� */
    SDK_U8                is_alarmout;
    SDK_U32               duration;   /* �����������ʱ�䣬��*/

    /* ���������� */
    SDK_U8                is_beep;   /* �Ƿ���������� */
    SDK_U32               beepTime;  /* ���������ʱ�䣬��*/

    /* ������̨��� */
    SDK_U8                is_ptz;
    SDK_U8                is_pushMsg;

    GK_PTZ_LINK           ptzLink;

    SDK_U8                alarmAudioType; /* ������������*/
    SDK_U8                is_light[4];       /* �׹��*/
    SDK_U8                bCheckedFigure; /* ���μ��*/
    SDK_U8                colorModeTime; /* ����ҹ���л����ڣ���*/

    GK_ALARM_HD_CFG       humanDetection;    /* ���μ��*/
    GK_ALARM_MT_CFG       motionTracing;     /* �˶�׷��*/
	GK_ALARM_TD_CFG       transboundaryDetection;     /* Խ�����*/
    GK_ALARM_RI_CFG       regionIntrusion;     /* ��������*/
} GK_HANDLE_EXCEPTION;


/*��������*/
typedef struct {
    SDK_S32            channel;
    SDK_S8             alarmInName[MAX_STR_LEN_32];    /* ���� */
    SDK_S8             defaultState;       /* 0-low 1-high */
    SDK_S8             activeState;        /* 0-low 1-high*/
    GK_HANDLE_EXCEPTION handle;               /* ����ʽ */
    GK_SCHEDTIME        scheduleTime[7][4];   /**��ͨ����videoloss�Ĳ���ʱ��*/
} GK_NET_ALARMINCFG, *LPGK_NET_ALARMINCFG;

/*�������*/
typedef struct {
    SDK_S32   channel;
    SDK_S8    alarmOutName[MAX_STR_LEN_32];   /* ���� */
    SDK_S8    defaultState;       /* 0-low 1-high */
    SDK_S8    activeState;        /* 0-low 1-high*/
    SDK_S8    powerOnState;       /* 0-pulse 1-continuous*/
    SDK_S32   pulseDuration;      /* 1000 - 10000*/
    GK_SCHEDTIME scheduleTime[7][4]; /**��ͨ����videoloss�Ĳ���ʱ��*/
} GK_NET_ALARMOUTCFG, *LPGK_NET_ALARMOUTCFG;

/*��������*/
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

/*ħ��4.0����*/
typedef struct {
	SDK_S8  gbStatus;   //����澯��Ϣ�������״̬��1������2���أ�
	SDK_S8  audioAlarmStatus;   //�����澯����״̬��1������2���أ�-1:��֧�֣�
	SDK_S8  audioFilesCur[128];
	SDK_S8  audioPlayCnt;

	SDK_S8 lightAlarmStatus;    //�澯�ƹ⿪��״̬��1������2���أ�-1:��֧�֣�
	SDK_S8 lightAlarmType;      //�ƹ�澯���ͣ�1=������2=��˸
	SDK_S8 lightAlarmFrequency; //��˸Ƶ�ʣ�1=�ߣ�2=�У�3=��
	SDK_S8 lightAlarmTime;      //ͣ��ʱ�䣨��λs������Χ5-30s��
} GK_MOJING_ALRAM;

typedef struct {
    GK_NET_ALARMINCFG  alarmIn;
    GK_NET_ALARMOUTCFG alarmOut;
	GK_ALARMCFG        cfg;
    
	SDK_S8              audioFiles[384];
    GK_MOJING_ALRAM     alarmInfo[2]; //0:�ƶ���� 1:��������
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


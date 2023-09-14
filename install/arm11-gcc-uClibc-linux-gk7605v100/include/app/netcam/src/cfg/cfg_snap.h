/*!
*****************************************************************************
** FileName     : cfg_snap.h
**
** Description  : config for snap.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_SNAP_H__
#define _GK_CFG_SNAP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***         snap                ***/
/***********************************/
/* ��ʱץͼ */
typedef struct {
    SDK_S32     enable;
    GK_SCHEDTIME scheduleTime[7][4];/**��ͨ����videoloss�Ĳ���ʱ��*/
    SDK_S32     interval;           // ��msһ��
    SDK_S32     nums;               // һ�μ���
    SDK_S32     pictureQuality;     // ����ֵ 0- 100 ��100% ���
    SDK_S32     imageSize;          // �����С��0: 1080p 1: 720p 2 D1 3��CIF��4��QCIF
    SDK_S32     snapShotImageType;  // 0 JPEG��1 bmp
    SDK_S32     storagerMode;       //�洢ģʽ(0:����, 1:FTP, 2: FTP|LOCAL(����FTP,FTPʧ�ܺ�¼�񱾵�))
    SDK_S32     channelID;          //ͨ����
} GK_TIMER_SNAP;

/*�¼�ץͼ*/
typedef struct {
	SDK_S32     enable;
	SDK_S32     interval;           //ץ�ļ��ms
	SDK_S32     nums;               // һ�μ���
	SDK_S32     pictureQuality;     // ����ֵ 0- 100 ��100% ���
	SDK_S32     imageSize;          // �����С
	SDK_S32     snapShotImageType;  // 0 JPEG��1 bmp
	SDK_S32     storagerMode;       //�洢ģʽ(0:����
	SDK_S32     channelID;          //ͨ����
}GK_EVENT_SNAP;

typedef struct {
	GK_TIMER_SNAP timer_snap;
	GK_EVENT_SNAP event_snap;
} GK_NET_SNAP_CFG, *LPGK_NET_SNAP_CFG;

extern int SnapCfgSave();
extern int SnapCfgLoad();
extern void SnapCfgPrint();
extern int SnapCfgLoadDefValue();

#define SNAP_CFG_FILE "gk_snap_cfg.cjson"


extern GK_NET_SNAP_CFG runSnapCfg;
extern GK_CFG_MAP snapTimerMap[];
extern GK_CFG_MAP snapEventMap[];

#ifdef __cplusplus
}
#endif
#endif


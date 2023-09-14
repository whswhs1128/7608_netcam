/*!
*****************************************************************************
** FileName     : cfg_record.h
**
** Description  : config for record.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_RECORD_H__
#define _GK_CFG_RECORD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***         record              ***/
/***********************************/
typedef struct {
	SDK_S32 	 sdCardFormatFlag;	   //�Ƿ��ʽ��SDcard  0:����ʽ��	 1:sdCard����ʱ�Զ�����ʽ��sd��  Ĭ��0
    SDK_S32      enable;             /*�Ƿ�¼�� 0-�� 1-��*/
    //SDK_S32      channel;            /* ѡ���ĸ�ͨ��*/
    SDK_S32      stream_no;            /* ѡ���ĸ�ͨ����ʼ¼�� 0-3 */
    SDK_S32      recordMode;         //  0:��Ԥ��ʱ��¼�� 1:Ԥ��ȫ��¼�� 2:�ֶ�¼��ģʽ 3:ֹͣ¼��
    GK_SCHEDTIME  scheduleTime[7][4]; //¼��ʱ��Σ�����һ��������
    SDK_U32       scheduleSlice[7][3]; /*��ͨ���Ĳ���ʱ��Ƭ��*/
    SDK_S32      preRecordTime;      /* Ԥ¼ʱ�䣬��λ��s��0��ʾ��Ԥ¼�� */
    SDK_S32      audioRecEnable;     /*¼��ʱ����������ʱ�Ƿ��¼��Ƶ����*/  
    SDK_S32      recAudioType;     /*¼�Ƶ���Ƶ��ʽ 0 a-law; 1 u-law; 2 pcm; 3-adpcm*/ 
    SDK_S32      recordLen;          //¼���ļ����ʱ��,�Է���Ϊ��λ
    SDK_S32      recycleRecord;      //�Ƿ�ѭ��¼��,0:����; 1:��
    SDK_S8       mojingRecTime[7][256];
} GK_NET_RECORD_CFG, *LPGK_NET_RECORD_CFG;

extern int RecordCfgSave();
extern int RecordCfgLoad();
extern void RecordCfgPrint();
extern int RecordCfgLoadDefValue();

#define RECORD_CFG_FILE "gk_record_cfg.cjson"

extern GK_NET_RECORD_CFG runRecordCfg;
extern GK_CFG_MAP recordMap[];

#ifdef __cplusplus
}
#endif
#endif


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
	SDK_S32 	 sdCardFormatFlag;	   //是否格式化SDcard  0:不格式化	 1:sdCard错误时自动化格式化sd卡  默认0
    SDK_S32      enable;             /*是否录像 0-否 1-是*/
    //SDK_S32      channel;            /* 选择哪个通道*/
    SDK_S32      stream_no;            /* 选择哪个通道开始录像 0-3 */
    SDK_S32      recordMode;         //  0:按预设时间录像 1:预设全天录像 2:手动录像模式 3:停止录像
    GK_SCHEDTIME  scheduleTime[7][4]; //录像时间段，星期一到星期天
    SDK_U32       scheduleSlice[7][3]; /*该通道的布防时间片段*/
    SDK_S32      preRecordTime;      /* 预录时间，单位是s，0表示不预录。 */
    SDK_S32      audioRecEnable;     /*录像时复合流编码时是否记录音频数据*/  
    SDK_S32      recAudioType;     /*录制的音频格式 0 a-law; 1 u-law; 2 pcm; 3-adpcm*/ 
    SDK_S32      recordLen;          //录像文件打包时长,以分钟为单位
    SDK_S32      recycleRecord;      //是否循环录像,0:不是; 1:是
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


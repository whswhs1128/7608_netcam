/*!
*****************************************************************************
** FileName     : cfg_audio.h
**
** Description  : config for audio.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_AUDIO_H__
#define _GK_CFG_AUDIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"
#include "sdk_audio.h"

/***********************************/
/***       audio                 ***/
/***********************************/


typedef struct {
    SDK_S32                 mode;           // 0 disable; 1 input; 2- input&output
    SDK_S32                 type;           // 0 a-law; 1 u-law; 2 pcm; 3-adpcm
    SDK_S32                 chans;          // 1
    EM_GK_AUDIO_SAMPLE_RATE   sampleRate;     //8000
    SDK_S32                 sampleBitWidth; //8, 16
    SDK_S32                 inputVolume;    // 100
    SDK_S32                 outputVolume;   // 100   
    SDK_S32                 rebootMute;           // 0 enable audio when boot; 1 disable audio
    SDK_U8                 audioInEnable;   //1=¹Ø±Õ£¬2=¿ªÆô
    SDK_U8                 audioOutEnable;
} GK_NET_AUDIO_CFG, *lPGK_NET_AUDIO_CFG;

extern int AudioCfgSave();
extern int AudioCfgLoad();
extern void AudioCfgPrint();
extern int AudioCfgLoadDefValue();

#define AUDIO_CFG_FILE "gk_audio_cfg.cjson"

extern GK_NET_AUDIO_CFG runAudioCfg;
extern GK_CFG_MAP audioMap[];

#ifdef __cplusplus
}
#endif
#endif


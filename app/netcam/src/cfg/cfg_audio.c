/*!
*****************************************************************************
** FileName     : cfg_audio.c
**
** Description  : audio config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_audio.h"

GK_NET_AUDIO_CFG runAudioCfg;

#if USE_DEFUALT_PARAM
GK_CFG_MAP audioMap[] = {
    {"mode",           &runAudioCfg.mode,           GK_CFG_DATA_TYPE_S32, "2",    "rw", 0, 2, "0 disable; 1 input; 2 input&output"},
    {"type",           &runAudioCfg.type,           GK_CFG_DATA_TYPE_S32, "0",    "rw", 0, 2, "0 a-law; 1 u-law; 2 PCM"},
    {"chans",          &runAudioCfg.chans,          GK_CFG_DATA_TYPE_S32, "2",    "rw", 1, 2, "1, 2"},
    {"sampleRate",     &runAudioCfg.sampleRate,     GK_CFG_DATA_TYPE_S32, "8000", "rw", 8000, 32000, NULL},
    {"sampleBitWidth", &runAudioCfg.sampleBitWidth, GK_CFG_DATA_TYPE_S32, "16",   "rw", 8, 32, NULL},
    {"inputVolume",    &runAudioCfg.inputVolume,    GK_CFG_DATA_TYPE_S32, "50",   "rw", 0, 100, NULL},
    {"outputVolume",   &runAudioCfg.outputVolume,   GK_CFG_DATA_TYPE_S32, "60",   "rw", 0, 100, NULL},
    {"rebootMute",     &runAudioCfg.rebootMute,     GK_CFG_DATA_TYPE_S32, "0",    "rw", 0, 1, "0 not mute when reboot; 1 mute when reboot"},
    {"audioInEnable",  &runAudioCfg.audioInEnable,  GK_CFG_DATA_TYPE_U8,  "1",    "rw", 0, 2, NULL},
    {"audioOutEnable", &runAudioCfg.audioOutEnable, GK_CFG_DATA_TYPE_U8,  "1",    "rw", 0, 2, NULL},
    {NULL,},   
};

#else
GK_CFG_MAP audioMap[] = {
    {"mode",           &runAudioCfg.mode,           },
    {"type",           &runAudioCfg.type,           },
    {"chans",          &runAudioCfg.chans,          },
    {"sampleRate",     &runAudioCfg.sampleRate,     },
    {"sampleBitWidth", &runAudioCfg.sampleBitWidth, },
    {"inputVolume",    &runAudioCfg.inputVolume,    },
    {"outputVolume",   &runAudioCfg.outputVolume,   },
    //{"rebootMute",      &runAudioCfg.rebootMute,    GK_CFG_DATA_TYPE_S32, "0",    "rw", 0, 1, "0 not mute when reboot; 1 mute when reboot"},
    {"rebootMute",      &runAudioCfg.rebootMute,    },
	{"audioInEnable",  &runAudioCfg.audioInEnable,	GK_CFG_DATA_TYPE_U8,  "1",	  1, 0, 2, NULL},
	{"audioOutEnable", &runAudioCfg.audioOutEnable, GK_CFG_DATA_TYPE_U8,  "1",	  1, 0, 2, NULL},
    {NULL,},
};

#endif

void AudioCfgPrint()
{
    printf("********** Audio *********\n");
    CfgPrintMap(audioMap);
    printf("********** Audio *********\n\n");
}

int AudioCfgSave()
{
    int ret = CfgSave(AUDIO_CFG_FILE, "audio", audioMap);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", AUDIO_CFG_FILE);
        return -1;
    }

    return 0;
}

int AudioCfgLoad()
{
    int ret = CfgLoad(AUDIO_CFG_FILE, "audio", audioMap);
    if (ret != 0) {
        PRINT_ERR("CfgLoad %s error.", AUDIO_CFG_FILE);
        return -1;
    }

    return 0;
}

int AudioCfgLoadDefValue()
{
    CfgLoadDefValue(audioMap);

    return 0;
}



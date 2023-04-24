/*!
*****************************************************************************
** FileName     : cfg_ptz.c
**
** Description  : ptz config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-5
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#include "cfg_ptz.h"
#include "utility_api.h"

GK_NET_PTZ_CFG     runPtzCfg;
GK_NET_DECODERCFG  runPtzDecCfg;

GK_NET_PRESET_INFO runPresetCfg;
GK_NET_CRUISE_CFG  runCruiseCfg;

#if USE_DEFAULT_PARAM
GK_CFG_MAP ptzMap[] = {
    {"id",           &runPtzCfg.id,           GK_CFG_DATA_TYPE_S32, "1", 1, 0, 10, NULL},
    {"serialPortID", &runPtzCfg.serialPortID, GK_CFG_DATA_TYPE_S32, "2", 1, 0, 10, NULL},
    {"videoInputID", &runPtzCfg.videoInputID, GK_CFG_DATA_TYPE_S32, "1", 1, 0, 10, NULL},
    {"duplexMode",   &runPtzCfg.duplexMode,   GK_CFG_DATA_TYPE_S32, "0", 1, 0, 1,  "0-half, 1-full"},
    {"controlType",  &runPtzCfg.controlType,  GK_CFG_DATA_TYPE_S32, "1", 1, 0, 1,  "0- controlType, 1- external"},
    {"protocol",     &runPtzCfg.protocol,     GK_CFG_DATA_TYPE_S32, "0", 1, 0, 10, "0 pelco-d, 1 pelco-p"},
    {"address",      &runPtzCfg.address,      GK_CFG_DATA_TYPE_S32, "1", 1, 0, 10, NULL},

    {"channel",        &runPtzDecCfg.channel,        GK_CFG_DATA_TYPE_S32,    "0",    1,  0, 255,  NULL},
    {"baudRate",       &runPtzDecCfg.baudRate,       GK_CFG_DATA_TYPE_S32, "1200",    1, 600, 115200, NULL},
    {"dataBit",        &runPtzDecCfg.dataBit,        GK_CFG_DATA_TYPE_U8,     "3",    1,  0,   3,  NULL},
    {"stopBit",        &runPtzDecCfg.stopBit,        GK_CFG_DATA_TYPE_U8,     "0",    1,  0,   2,  NULL},
    {"parity",         &runPtzDecCfg.parity,         GK_CFG_DATA_TYPE_U8,     "0",    1,  0,   2,  NULL},
    {"flowcontrol",    &runPtzDecCfg.flowcontrol,    GK_CFG_DATA_TYPE_U8,     "0",    1,  0,   2,  NULL},
    {"workMode",       &runPtzDecCfg.workMode,       GK_CFG_DATA_TYPE_S32,    "2",    1,  0,   2,  NULL},
    {"decoderAddress", &runPtzDecCfg.decoderAddress, GK_CFG_DATA_TYPE_U16,    "0",    1,  0, 255,  NULL},
    {"speedH",         &runPtzDecCfg.speedH,         GK_CFG_DATA_TYPE_U16,    "2",    1,  0,  64,  NULL},
    {"speedV",         &runPtzDecCfg.speedV,         GK_CFG_DATA_TYPE_U16,    "2",    1,  0,  64,  NULL},
    {"watchPos",       &runPtzDecCfg.watchPos,       GK_CFG_DATA_TYPE_U16,    "0",    1,  0, 255,  NULL},
    {"decoderType",    &runPtzDecCfg.decoderType,    GK_CFG_DATA_TYPE_STRING, "pelco-d",  1,  1, MAX_STR_LEN_32,  NULL},

    {NULL,},
};


#else
GK_CFG_MAP ptzMap[] = {
    {"id",           &runPtzCfg.id,           },
    {"serialPortID", &runPtzCfg.serialPortID, },
    {"videoInputID", &runPtzCfg.videoInputID, },
    {"duplexMode",   &runPtzCfg.duplexMode,   },
    {"controlType",  &runPtzCfg.controlType,  },
    {"protocol",     &runPtzCfg.protocol,     },
    {"address",      &runPtzCfg.address,      },

    {"channel",        &runPtzDecCfg.channel,        },
    {"baudRate",       &runPtzDecCfg.baudRate,       },
    {"dataBit",        &runPtzDecCfg.dataBit,        },
    {"stopBit",        &runPtzDecCfg.stopBit,        },
    {"parity",         &runPtzDecCfg.parity,         },
    {"flowcontrol",    &runPtzDecCfg.flowcontrol,    },
    {"workMode",       &runPtzDecCfg.workMode,       },
    {"decoderAddress", &runPtzDecCfg.decoderAddress, },
    {"speedH",         &runPtzDecCfg.speedH,         },
    {"speedV",         &runPtzDecCfg.speedV,         },
    {"watchPos",       &runPtzDecCfg.watchPos,       },
    {"decoderType",    &runPtzDecCfg.decoderType,    },

    {NULL,},
};
#endif


void PtzCfgPrint()
{
    printf("********** Ptz *********\n");
    CfgPrintMap(ptzMap);
    printf("********** Ptz *********\n\n");
}

int PtzCfgSave()
{
    int ret = CfgSave(PTZ_CFG_FILE, "ptz", ptzMap);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", PTZ_CFG_FILE);
        return -1;
    }

    return 0;
}

int PtzCfgLoad()
{
    int ret = CfgLoad(PTZ_CFG_FILE, "ptz", ptzMap);
    if (ret != 0) {
        PRINT_ERR("CfgLoad %s error.", PTZ_CFG_FILE);
        return -1;
    }

    return 0;
}

int PtzCfgLoadDefValue()
{
    CfgLoadDefValue(ptzMap);

    return 0;
}


int PresetCruiseCfgDefault()
{
    int i, j;

    memset(&runPresetCfg, 0, sizeof(runPresetCfg));
    memset(&runCruiseCfg, 0, sizeof(runCruiseCfg));

    for (i = 0; i < PTZ_MAX_CRUISE_GROUP_NUM; i++)
    {
        runCruiseCfg.struCruise[i].byPointNum    = 0;
        runCruiseCfg.struCruise[i].byCruiseIndex = i;
        for (j = 0; j < PTZ_MAX_CRUISE_POINT_NUM; j++)
        {
            runCruiseCfg.struCruise[i].struCruisePoint[j].byPointIndex = 0;
            runCruiseCfg.struCruise[i].struCruisePoint[j].byPresetNo   = 0;
            runCruiseCfg.struCruise[i].struCruisePoint[j].byRemainTime = 0;
            runCruiseCfg.struCruise[i].struCruisePoint[j].bySpeed      = 0;
        }
    }

    return 0;
}


int PresetCruiseCfgSave()
{
    int ret = 0;
    int fd  = -1;
    int val = 0;

    if (access(PTZ_CRUISE_CFG, F_OK) != 0)
    {
        PRINT_ERR("File:%s don't exist! creat new file. \n", PTZ_CRUISE_CFG);
        fd = open(PTZ_CRUISE_CFG, (O_CREAT|O_RDWR|O_TRUNC));
        if (fd < 0)
        {
            PRINT_ERR("open %s ERROR! %s\n", PTZ_CRUISE_CFG, strerror(errno));
            return -1;
        }
    }
    else
    {
        fd = open(PTZ_CRUISE_CFG, O_RDWR);
        if (fd < 0)
        {
            PRINT_ERR("open %s ERROR! %s\n", PTZ_CRUISE_CFG, strerror(errno));
            return -1;
        }
    }

    val = utility_crc32(0,   &runPresetCfg, sizeof(runPresetCfg));
    val = utility_crc32(val, &runCruiseCfg, sizeof(runCruiseCfg));

    PRINT_INFO("FUN[%s]  LINE[%d]  crc_val:0x%x \n", __FUNCTION__, __LINE__, val);

    ret = write(fd, &val, sizeof(val));
    if (ret <= 0)
    {
        PRINT_ERR("write CRC_val error! %s\n", strerror(errno));
        close(fd);
    }

    ret = write(fd, &runPresetCfg, sizeof(runPresetCfg));
    if (ret <= 0)
    {
        PRINT_ERR("write runPresetCfg error! %s\n", strerror(errno));
        close(fd);
    }

    ret = write(fd, &runCruiseCfg, sizeof(runCruiseCfg));
    if (ret <= 0)
    {
        PRINT_ERR("write runCruiseCfg error! %s\n", strerror(errno));
        close(fd);
    }

    close(fd);
    return 0;
}


int PresetCruiseCfgLoad()
{
    int ret = 0;
    int crc = 0;
    int val = 0;
    int fd  = -1;

    if (access(PTZ_CRUISE_CFG, F_OK) != 0)
    {
        PRINT_ERR("File:%s don't exist! creat new file. \n", PTZ_CRUISE_CFG);
        PresetCruiseCfgDefault();
        PresetCruiseCfgSave();
        return 0;
    }
    else
    {
        fd = open(PTZ_CRUISE_CFG, O_RDWR);
        if (fd < 0)
        {
            PRINT_ERR("open %s ERROR! %s\n", PTZ_CRUISE_CFG, strerror(errno));
            return -1;
        }
    }

    ret = read(fd, &crc, sizeof(crc));
    if (ret <= 0)
    {
        PRINT_ERR("read CRC_val error! %s\n", strerror(errno));
        close(fd);
    }

    ret = read(fd, &runPresetCfg, sizeof(runPresetCfg));
    if (ret <= 0)
    {
        PRINT_ERR("read runPresetCfg error! %s\n", strerror(errno));
        close(fd);
    }


    ret = read(fd, &runCruiseCfg, sizeof(runCruiseCfg));
    if (ret <= 0)
    {
        PRINT_ERR("read runCruiseCfg error! %s\n", strerror(errno));
        close(fd);
    }
    close(fd);

    val = utility_crc32(0,   &runPresetCfg, sizeof(runPresetCfg));
    val = utility_crc32(val, &runCruiseCfg, sizeof(runCruiseCfg));

    if (val != crc)
    {
        PRINT_ERR("Get check crc error! val:0x%x  crc:0x%x\n", val, crc);
        PresetCruiseCfgDefault();
        PresetCruiseCfgSave();
        return 0;
    }

    return 0;
}


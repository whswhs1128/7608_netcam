/*!
*****************************************************************************
** FileName     : cfg_image.c
**
** Description  : image config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-1
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_image.h"

GK_NET_IMAGE_CFG runImageCfg;


#if USE_DEFAULT_PARAM
GK_CFG_MAP imageMap[] = {
    {"sceneMode",     &runImageCfg.sceneMode,     GK_CFG_DATA_TYPE_U8, "0", 1, 0, 2, "0 auto 1 indoor 2 outdoor"},
    {"imageStyle",    &runImageCfg.imageStyle,    GK_CFG_DATA_TYPE_U8, "0", 1, 0, 2, "0 nomal 1 lightness 2 bright"},
    {"wbMode",        &runImageCfg.wbMode,        GK_CFG_DATA_TYPE_U8, "0", 1, 0, 2,  "0 auto 1 indoor 2 outdoor"},

    {"irCutControlMode",  &runImageCfg.irCutControlMode,  GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, "0 hardware,  1 software"},
    {"irCutMode",         &runImageCfg.irCutMode,         GK_CFG_DATA_TYPE_U8, "0", 1, 0, 2, "0 auto, 1 day, 2 night"},
    {"enabledWDR",        &runImageCfg.enabledWDR,        GK_CFG_DATA_TYPE_U8, "1", 1, 0, 1, NULL},
    {"strengthWDR",       &runImageCfg.strengthWDR,       GK_CFG_DATA_TYPE_U8, "3", 1, 1, 5, NULL},
    {"enableDenoise3d",   &runImageCfg.enableDenoise3d,   GK_CFG_DATA_TYPE_U8, "1", 1, 0, 1, NULL},
    {"strengthDenoise3d", &runImageCfg.strengthDenoise3d, GK_CFG_DATA_TYPE_U8, "1", 1, 0, 100, NULL},

    {"lowlightMode",    &runImageCfg.lowlightMode,    GK_CFG_DATA_TYPE_U8,  "3", 1, 0, 3, "0 close, 1 only night, 2 day-night, 3 auto"},
    {"exposureMode",    &runImageCfg.exposureMode,    GK_CFG_DATA_TYPE_U8,  "0", 1, 0, 2, "0 - auto 1 - bright, 2 - dark"},
    {"dcIrisEnable",    &runImageCfg.dcIrisEnable,    GK_CFG_DATA_TYPE_U8,  "1", 1, 0, 1, NULL},
    {"antiFlickerFreq", &runImageCfg.antiFlickerFreq, GK_CFG_DATA_TYPE_U8,  "50", 1, 50, 60, "50HZ 60HZ"},
    {"backLightEnable", &runImageCfg.backLightEnable, GK_CFG_DATA_TYPE_U8,  "1", 1, 0, 1, NULL},
    {"backLightLevel",  &runImageCfg.backLightLevel,  GK_CFG_DATA_TYPE_S32, "3", 1, 0, 100, NULL},

    {"brightness",  &runImageCfg.brightness, GK_CFG_DATA_TYPE_S32, "50", 1, 0, 100, NULL},
    {"saturation",  &runImageCfg.saturation, GK_CFG_DATA_TYPE_S32, "50", 1, 0, 100, NULL},
    {"contrast",    &runImageCfg.contrast,   GK_CFG_DATA_TYPE_S32, "50", 1, 0, 100, NULL},
    {"sharpness",   &runImageCfg.sharpness,  GK_CFG_DATA_TYPE_S32, "50", 1, 0, 100, NULL},
    {"hue",         &runImageCfg.hue,        GK_CFG_DATA_TYPE_S32, "50",   1, 0, 100, NULL},
    {"flipEnabled",   &runImageCfg.flipEnabled,   GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"mirrorEnabled", &runImageCfg.mirrorEnabled, GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},

    {NULL,},

};
#else
GK_CFG_MAP imageMap[] = {
    {"sceneMode",           &runImageCfg.sceneMode,             },
    {"imageStyle",          &runImageCfg.imageStyle,            },
    {"wbMode",              &runImageCfg.wbMode,                },

    {"irCutControlMode",    &runImageCfg.irCutControlMode,      },
    {"irCutMode",           &runImageCfg.irCutMode,             },
    {"enabledWDR",          &runImageCfg.enabledWDR,            },
    {"strengthWDR",         &runImageCfg.strengthWDR,           },
    {"enableDenoise3d",     &runImageCfg.enableDenoise3d,       },
    {"strengthDenoise3d",   &runImageCfg.strengthDenoise3d,     },

    {"lowlightMode",        &runImageCfg.lowlightMode,          },
    {"exposureMode",        &runImageCfg.exposureMode,          },
    {"dcIrisEnable",        &runImageCfg.dcIrisEnable,          },
    {"antiFlickerFreq",     &runImageCfg.antiFlickerFreq,       },
    {"backLightEnable",     &runImageCfg.backLightEnable,       },
    {"backLightLevel",      &runImageCfg.backLightLevel,        },

    {"brightness",          &runImageCfg.brightness,            },
    {"saturation",          &runImageCfg.saturation,            },
    {"contrast",            &runImageCfg.contrast,              },
    {"sharpness",           &runImageCfg.sharpness,             },
    {"hue",                 &runImageCfg.hue,                   },
    {"flipEnabled",         &runImageCfg.flipEnabled,           },
    {"mirrorEnabled",       &runImageCfg.mirrorEnabled,         },

	/*lightMode & lightLevel used by CUS_HC_GK7202_GC2053_V10 board */
    {"lightMode",           &runImageCfg.lightMode,             GK_CFG_DATA_TYPE_U8, "0", 1, 0, 3, NULL},
    {"lightLevel",          &runImageCfg.lightLevel,            GK_CFG_DATA_TYPE_U8, "0", 1, 0, 100, NULL},

    {"led1",                &runImageCfg.led1,             GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {"led2",                &runImageCfg.led2,            GK_CFG_DATA_TYPE_U8, "0", 1, 0, 1, NULL},
    {NULL,},

};

#endif


void ImageCfgPrint()
{
    printf("********** Image *********\n");
    CfgPrintMap(imageMap);
    printf("********** Image *********\n\n");
}

int ImageCfgSave()
{
    int ret = CfgSave(IMAGE_CFG_FILE, "image", imageMap);
    if (ret != 0) {
        PRINT_ERR("CfgSave %s error.", IMAGE_CFG_FILE);
        return -1;
    }

    return 0;
}

int ImageCfgLoad()
{
    int ret = CfgLoad(IMAGE_CFG_FILE, "image", imageMap);
    if (ret != 0) {
        PRINT_ERR("CfgLoad %s error.", IMAGE_CFG_FILE);
        return -1;
    }

    return 0;
}

int ImageCfgLoadDefValue()
{
    CfgLoadDefValue(imageMap);

    return 0;
}

char *ImageCfgGetJosnString(void)
{

    char *out;
    cJSON *root;

    root = CfgDataToCjsonByMap(imageMap);
    if(root == NULL)
    {
        PRINT_ERR("imageMap to json error.");
        return NULL;
    }
    out = cJSON_Print(root);
    //PRINT_INFO("Image json:%s\n",out);
    cJSON_Delete(root);
    return out;

}

